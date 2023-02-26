#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

#include "jaam.h"

#include "nlohmann/json.hpp"
#include "lz4.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "assetTexture.h"

#include "util.h"
#include "modelConverter.h"
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <chrono>
#include <unordered_set>

using namespace Asset;


static uint16_t textureChecksum = 0;

namespace
{
	class JobPool
	{
	public:
		JobPool(int threadCount);
		~JobPool();
		void push(std::function<void()> func);
		void done();
	private:

		void work();

		std::queue<std::function<void()>> m_function_queue;
		std::mutex m_lock;
		std::condition_variable m_data_condition;
		std::atomic<bool> m_accept_functions;
		std::vector<std::thread> m_workerThreads;
	};

	JobPool::JobPool(int threadCount) : m_function_queue(), m_lock(), m_data_condition(), m_accept_functions(true)
	{
		m_workerThreads.resize(threadCount);
		for (uint32_t  i = 0; i < threadCount; i++)
		{
			m_workerThreads.at(i) = std::thread(&JobPool::work, this);
		}
	}

	JobPool::~JobPool()
	{
	}

	void JobPool::push(std::function<void()> func)
	{
		//If there are no worker threads just call the func (single threaded)
		if (m_workerThreads.empty())
			func();

		std::unique_lock<std::mutex> lock(m_lock);
		m_function_queue.push(func);
		// when we send the notification immediately, the consumer will try to get the lock , so unlock asap
		lock.unlock();
		m_data_condition.notify_one();
	}

	void JobPool::done()
	{
		std::unique_lock<std::mutex> lock(m_lock);
		m_accept_functions = false;
		lock.unlock();
		// when we send the notification immediately, the consumer will try to get the lock , so unlock asap
		m_data_condition.notify_all();
		//notify all waiting threads.

		for (auto& thread : m_workerThreads)
		{
			thread.join();
		}
	}

	void JobPool::work()
	{
		std::function<void()> func;
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_lock);
				m_data_condition.wait(lock, [this]()
					{
						return !m_function_queue.empty() || !m_accept_functions;
					});

				if (!m_accept_functions && m_function_queue.empty())
				{
					//lock will be release automatically.
					//finish the thread loop and let it join in the main thread.
					return;
				}
				func = m_function_queue.front();
				m_function_queue.pop();
				//release the lock
			}
			func();
		}
	}
}


bool ConvertImage(const fs::path& input, const fs::path& output, const fs::path& rootPath)
{
	int texWidth, texHeight, texChannels;

	stbi_uc* pixels = stbi_load(input.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "Failed to load texture file " << input << std::endl;
		return false;
	}

	int texture_size = texWidth * texHeight * 4;

	TextureInfo texinfo;
	texinfo.textureSize = texture_size;
	texinfo.pixelsize[0] = texWidth;
	texinfo.pixelsize[1] = texHeight;
	texinfo.textureFormat = TextureFormat::RGBA8;
	texinfo.originalFile = GetRelativePathFrom(input, rootPath.string()).string();
	AssetFile newImage = PackTexture(&texinfo, pixels);
	newImage.checksum = textureChecksum++;

	stbi_image_free(pixels);

	newImage.SaveBinaryFile(output.string().c_str());

	return true;
}


int main(int argc, char** argv)
{
	std::unordered_set<std::string> textureExtensions =
	{
		".png",
		".jpg",
		".jpeg",
		".bmp"
	};

	std::unordered_set<std::string> modelExtensions =
	{
		".obj",
		".fbx",
		".gltf"
	};

	int num_threads = std::thread::hardware_concurrency();
	std::cout << "number of threads = " << num_threads << std::endl;
	JobPool jobPool(num_threads);

	for (int i = 0; i < argc; ++i)
		std::cout << argv[i] << '\n';

	const fs::path path{ argv[1] };
	const fs::path output{ argv[2] };

	std::cout << "loading asset directory at " << path << std::endl;

	auto start = std::chrono::high_resolution_clock::now();

	for (auto& p : fs::recursive_directory_iterator(path))
	{
		const fs::path rootPath = path.filename();
		fs::path newpath = ChangeRoot(path, output, p.path());
		fs::path newdir = newpath;

		fs::create_directories(newdir.remove_filename());

		if (textureExtensions.find(p.path().extension().string()) != textureExtensions.end()) {
			std::cout << "found a texture" << p << std::endl;

			newpath.replace_extension(".tx");
			jobPool.push([=]
				{
					ConvertImage(p.path(), newpath, rootPath);
				});
			
		}
		if (modelExtensions.find(p.path().extension().string()) != modelExtensions.end())
		{
			std::cout << "found a mesh" << p << std::endl;

			newpath.replace_extension(".mesh");
			jobPool.push([=]
				{
					ConvertMesh(p.path(), newpath, rootPath);
				});
		}
	}

	jobPool.done();

	auto end = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << microseconds.count() << "ms to package\n";

}