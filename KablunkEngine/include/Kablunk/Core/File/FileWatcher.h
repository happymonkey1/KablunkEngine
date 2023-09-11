#pragma once
#ifndef KABLUNK_CORE_FILE_WATCHER_H
#define KABLUNK_CORE_FILE_WATCHER_H

#include "Kablunk/Core/File/FileStatus.h"

#include <string>
#include <unordered_map>

#include <filesystem>

namespace kb
{
	// #TODO should this be separated into a FileWatcher and DirectoryWatcher?
	class FileWatcher
	{
	public:

		explicit FileWatcher(const std::filesystem::path& directory_to_watch);
		FileWatcher(const FileWatcher&) = delete;
		FileWatcher(FileWatcher&&);

		~FileWatcher();

		bool is_watching_path(const std::filesystem::path& path) const;
	private:
		bool m_running = true;

		std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> m_paths;
	};

}

#endif
