#pragma once
#ifndef KABLUNK_CORE_FILE_FILE_WATCHER_H
#define KABLUNK_CORE_FILE_FILE_WATCHER_H

namespace kb
{
	class FileWatcher;

	class FileWatcherFactory
	{
		static FileWatcher* Create();
	};
}

#endif
