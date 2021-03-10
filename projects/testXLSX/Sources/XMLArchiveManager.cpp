#include "XMLArchiveManager.h"
#include "ModuleFileManager.h"
#include "miniz.h"

XMLArchiveManager::XMLArchiveManager()
{

}
XMLArchiveManager::~XMLArchiveManager()
{
	clear();
}

// update folder hierarchy and return filename alone
XMLArchiveFolder*	XMLArchiveManager::updateFolderHierarchy(const std::string& path)
{
	// search all / in path and update mHierarchy

	size_t foundpos = path.find("/");
	size_t lastpos = 0;
	XMLArchiveFolder* current = &mRoot;
	while (foundpos != std::string::npos)
	{
		std::string foldername = path.substr(lastpos, foundpos - lastpos);
		XMLArchiveFolder* folder = current->findFolder(foldername);
		if (!folder)
		{
			folder = new XMLArchiveFolder(foldername,current);
			current->addSon(folder);
		}

		current = folder;

		lastpos = foundpos+1;
		foundpos = path.find("/", lastpos);
	}

	return current;
}

XMLArchiveHierarchy* XMLArchiveFolder::getFile(const std::string& n)
{

	size_t foundpos = n.find("/");
	if(foundpos != std::string::npos)
	{
		if (foundpos == 0) // if first char is "/" just remove it
		{
			return getFile(n.substr(1));
		}
		std::string foldername = n.substr(0, foundpos);
		XMLArchiveFolder* folder = findFolder(foldername);
		if (!folder)
		{
			return nullptr;
		}

		std::string remaining = n.substr(foundpos + 1);
		return folder->getFile(remaining);
	}
	else
	{
		for (auto& f : mSons)
		{
			if (n == f->getName())
			{
				return f;
			}
		}
	}

	return nullptr;
}

std::string XMLArchiveManager::getFilename(const std::string& fullpath)
{
	std::string fname = fullpath;
	size_t foundpos = fullpath.rfind("/");

	if (foundpos != std::string::npos)
	{
		fname = fname.substr(foundpos+1);
	}
	return fname;
}


bool	XMLArchiveManager::open(const std::string& filename)
{
	clear();
	u64 flen;
	CoreRawBuffer* buffer = ModuleFileManager::LoadFile(filename.c_str(), flen);
	if (buffer)
	{
		bool arOK = false;
		mz_zip_archive ar = {};
		if (mz_zip_reader_init_mem(&ar, buffer->buffer(), flen, 0))
		{
			arOK = true;
			u32 fcount = mz_zip_reader_get_num_files(&ar);
			
			for (u32 i=0; i < fcount; i++)
			{
				mz_zip_archive_file_stat stats = {};
				if (mz_zip_reader_file_stat(&ar, i, &stats))
				{
					XMLArchiveFolder* folder = updateFolderHierarchy(stats.m_filename);
					if (!stats.m_is_directory)
					{
						size_t filesize;
						void* data=mz_zip_reader_extract_to_heap(&ar,i, &filesize, 0);
						CoreRawBuffer* buf = new CoreRawBuffer();
						buf->SetBuffer(nullptr, filesize, true); // alloc buffer
						memcpy(buf->buffer(), data, filesize);
						free(data);

						std::string currentFilename = getFilename(stats.m_filename);

						XMLArchiveFile* toAdd = new XMLArchiveFile(currentFilename, buf, folder);

						// check the ones that you can immediatly convert to xml
						std::string ext = currentFilename.substr(currentFilename.length() - 4,4);
						if (ext == ".xml")
						{
							toAdd->interpretAsXML();
						}

						folder->addSon(toAdd);
					}
				}
			}
		}
		buffer->Destroy();
		return arOK;

	}
	return false;
}

bool	XMLArchiveFile::interpretAsXML()
{
	if (mRawData)
	{
		mXMLData = XMLReaderFile::ReadCoreRawBuffer(mRawData);
		if (mXMLData)
		{
			mRawData->Destroy();
			mRawData = nullptr;
			return true;
		}
	}
	return false;
}

void	XMLArchiveManager::clear()
{
	mRoot.clear();
}

// create an archive to be saved as a file (
CoreRawBuffer* XMLArchiveManager::save()
{
/*	mz_bool status;
	mz_zip_archive zip = {};
	if (status = mz_zip_writer_init_heap(&zip, 0, 0); !status) co_return false;
	*/
	// TODO
	return nullptr;
}