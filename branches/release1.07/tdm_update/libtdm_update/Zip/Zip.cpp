/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4751 $
 * $Date: 2011-04-07 03:06:49 -0400 (Thu, 07 Apr 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#include "Zip.h"

#include <time.h>
#include <fstream>
#include "minizip/unzip.h"
#include "minizip/zip.h"

#include "../Constants.h"
#include "../TraceLog.h"
#include "../File.h"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace tdm
{

	// Shortcut method
	inline std::string intToStr(int number)
	{
		return boost::lexical_cast<std::string>(number);
	}

ZipFileRead::ZipFileRead(unzFile handle) :
	_handle(handle)
{}

ZipFileRead::~ZipFileRead()
{
	unzClose(_handle);
}

void ZipFileRead::ForeachFile(Visitor& visitor)
{
	int result = unzGoToFirstFile(_handle);

	if (result != UNZ_OK)
	{
		throw std::runtime_error("[ForeachFile]: Cannot go to first file: " + intToStr(result));
	}

	while (result == UNZ_OK)
	{
		// get zipped file info
		unz_file_info info;
		char filenameBuf[4096];
		result = unzGetCurrentFileInfo(_handle, &info, filenameBuf, 4096, NULL, 0, NULL, 0);
	
		if (result != UNZ_OK)
		{
			throw std::runtime_error("[ForeachFile]: Cannot get file info: " + intToStr(result));
		}

		MemberInfo memberInfo;

		memberInfo.filename = filenameBuf;
		memberInfo.crc = info.crc;
		memberInfo.compressedSize = info.compressed_size;
		memberInfo.uncompressedSize = info.uncompressed_size;

		visitor.VisitFile(memberInfo);

		result = unzGoToNextFile(_handle);
	}
}

std::string ZipFileRead::GetGlobalComment()
{
	unz_global_info global_info;
	unzGetGlobalInfo(_handle, &global_info);

	if (global_info.size_comment == 0)
	{
		return "";
	}

	// size_comment > 0
	std::vector<char> buf(global_info.size_comment);
	unzGetGlobalComment(_handle, &buf.front(), buf.size());

	return std::string(buf.begin(), buf.end());
}

bool ZipFileRead::ContainsFile(const std::string& filename)
{
	int result = unzLocateFile(_handle, filename.c_str(), 0);

	return (result == UNZ_OK);
}

std::string ZipFileRead::LoadTextFile(const std::string& filename)
{
	int result = unzLocateFile(_handle, filename.c_str(), 0);

	if (result != UNZ_OK) return "";

	unz_file_info info;
	unzGetCurrentFileInfo(_handle, &info, NULL, 0, NULL, 0, NULL, 0);

	unsigned long fileSize = info.uncompressed_size;

	int openResult = unzOpenCurrentFile(_handle);

	std::string returnValue;

	if (openResult == UNZ_OK)
	{
		char* buffer = new char[fileSize + 1];

		// Read and null-terminate the string
		unzReadCurrentFile(_handle, buffer, fileSize);
		buffer[fileSize] = '\0';

		returnValue = buffer;
		
		delete[] buffer;
	}
	else
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[LoadTextFile]: Cannot open file. " + filename + ": " + intToStr(openResult));
	}

	unzCloseCurrentFile(_handle);

	return returnValue;
}

bool ZipFileRead::ExtractFileTo(const std::string& filename, const fs::path& destPath)
{
	bool returnValue = true;

	int result = unzLocateFile(_handle, filename.c_str(), 0);

	if (result != UNZ_OK) return false;

	// Make sure the destination file is not existing
	File::Remove(destPath);

	// Try to open the destination path before uncompressing the file
	FILE* outFile = fopen(destPath.file_string().c_str(), "wb");

	if (outFile == NULL) 
	{
		// couldn't open file for writing
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ExtractFileTo]: Cannot open destination file " + destPath.file_string());
		return false; 
	}

	unz_file_info info;
	result = unzGetCurrentFileInfo(_handle, &info, NULL, 0, NULL, 0, NULL, 0);

	if (result != UNZ_OK) 
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ExtractFileTo]: Cannot get file info for " + filename + ": " + intToStr(result));
		return false;
	}

	unsigned long fileSize = info.uncompressed_size;

	int openResult = unzOpenCurrentFile(_handle);

	if (openResult == UNZ_OK)
	{
		unsigned char* buffer = new unsigned char[fileSize];

		// Read and null-terminate the string
		unzReadCurrentFile(_handle, buffer, fileSize);

		fwrite(buffer, 1, fileSize, outFile);
				
		delete[] buffer;
	}
	else
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ExtractFileTo]: Cannot open file in zip " + filename + ": " + intToStr(openResult));
		returnValue = false; // fopen failed
	}

	fclose(outFile);

	result = unzCloseCurrentFile(_handle);

	if (result != UNZ_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[LoadTextFile]: Cannot close file in zip " + filename + ": " + intToStr(result));
	}

	return returnValue;
}

std::list<fs::path> ZipFileRead::ExtractAllFilesTo(const fs::path& destPath, 
												   const std::set<std::string>& ignoreIfExisting, 
												   const std::set<std::string>& ignoreList)
{
	int result = unzGoToFirstFile(_handle);

	if (result != UNZ_OK)
	{
		throw std::runtime_error("[ExtractAllFilesTo]: Cannot go to first file: " + intToStr(result));
	}

	std::vector<std::string> filesToExtract;

	while (result == UNZ_OK)
	{
		// get zipped file info
		unz_file_info info;
		char filenameBuf[4096];
		result = unzGetCurrentFileInfo(_handle, &info, filenameBuf, 4096, NULL, 0, NULL, 0);
	
		if (result != UNZ_OK)
		{
			throw std::runtime_error("[ExtractAllFilesTo]: Cannot get file info: " + intToStr(result));
		}

		std::string filename = filenameBuf;

		if (ignoreList.find(boost::algorithm::to_lower_copy(filename)) == ignoreList.end())
		{
			// File not on hard ignore list, check for "ignore if exists"
			
			if (ignoreIfExisting.find(boost::algorithm::to_lower_copy(filename)) != ignoreIfExisting.end() &&
				fs::exists(destPath / filename))
			{
				TraceLog::WriteLine(LOG_VERBOSE, "Ignoring file, as destination exists: " + filename);
			}
			else
			{
				TraceLog::WriteLine(LOG_VERBOSE, "Will extract file: " + filename);
				filesToExtract.push_back(filename);
			}
		}
		else
		{
			TraceLog::WriteLine(LOG_VERBOSE, "Ignoring file: " + filename);
		}

		result = unzGoToNextFile(_handle);
	}

	TraceLog::WriteLine(LOG_VERBOSE, "Found " + boost::lexical_cast<std::string>(filesToExtract.size()) + " files to extract.");

	// The list of extracted files, for returning to the caller
	std::list<fs::path> extractedFiles;

	for (std::size_t i = 0; i < filesToExtract.size(); ++i)
	{
		fs::path destFile = destPath / filesToExtract[i];

		ExtractFileTo(filesToExtract[i], destFile);

		extractedFiles.push_back(destFile);
	}

	return extractedFiles;
}

ZipFileRead::CompressedFilePtr ZipFileRead::ReadCompressedFile(const std::string& filename)
{
	int result = unzLocateFile(_handle, filename.c_str(), 0);

	if (result != UNZ_OK) 
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile]: File not in zip " + filename + ": " + intToStr(result));
		return CompressedFilePtr();
	}

	// Get the file information
	unz_file_info info;
	result = unzGetCurrentFileInfo(_handle, &info, NULL, 0, NULL, 0, NULL, 0);

	if (result != UNZ_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile]: Cannot get file info for " + filename + ": " + intToStr(result));
		return CompressedFilePtr();
	}

	// Create returned structure and initialise values
	CompressedFilePtr output(new CompressedFile);

	output->crc32 = info.crc;
	output->uncompressedSize = info.uncompressed_size;

	// Reserve space for the extra field and comment buffers
	output->extraField.resize(info.size_file_extra);
	output->comment.resize(info.size_file_comment);

	void* extraField = !output->extraField.empty() ? &output->extraField.front() : NULL;
	char* comment = !output->comment.empty() ? &output->comment.front() : NULL;

	result = unzGetCurrentFileInfo(_handle, &info, NULL, 0, extraField, output->extraField.size(), comment, output->comment.size());

	// NULL-terminate the comment
	if (!output->comment.empty())
	{
		output->comment.push_back('\0');
	}
	
	// Copy time field
	tm changeTime;
	changeTime.tm_hour = info.tmu_date.tm_hour;
	changeTime.tm_min = info.tmu_date.tm_min;
	changeTime.tm_sec = info.tmu_date.tm_sec;

	changeTime.tm_mday = info.tmu_date.tm_mday;
	changeTime.tm_mon = info.tmu_date.tm_mon;
	changeTime.tm_year = info.tmu_date.tm_year - 1900;

	output->changeTime = mktime(&changeTime);
	
	// Open the file for raw read
	int method;
	int level;
	result = unzOpenCurrentFile2(_handle, &method, &level, 1);

	if (result != UNZ_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile]: Cannot open file info for raw read " + filename + ": " + intToStr(result));
		return CompressedFilePtr();
	}

	// Remember the compression method and level
	output->compressionMethod = method == Z_DEFLATED ? CompressedFile::DEFLATED : CompressedFile::STORED;
	output->compressionLevel = level;

	// this malloc may fail if the compressed data is very large
	output->data.resize(info.compressed_size);
	
	if (output->data.size() != info.compressed_size) 
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile]: Could not allocate memory for " + filename + ": " + 
								 intToStr(result) + " - Size: " + boost::lexical_cast<std::string>(info.compressed_size));
		return CompressedFilePtr();
	}

	// read file
	void* data = !output->data.empty() ? &output->data.front() : NULL;
	int bytesRead = unzReadCurrentFile(_handle, data, info.compressed_size);

	if (bytesRead < 0)
	{
		// Return value negative, this is an error
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile] Error: unzReadCurrentFile returned error code: " + intToStr(bytesRead));
		return CompressedFilePtr();
	}
	else if (static_cast<uLong>(bytesRead) != info.compressed_size) 
	{
		// Bytes read != bytes claimed
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[ReadCompressedFile] Error: Bytes read != compressed size, bailing out: " + filename);
		return CompressedFilePtr();
	}

	// 50 kB Max. local header size
	int localHeaderSize = unzGetLocalExtrafield(_handle, NULL, 0);

	if (localHeaderSize > 0)
	{
		output->localExtraField.resize(localHeaderSize);
		int bytesCopied = unzGetLocalExtrafield(_handle, &output->localExtraField.front(), output->localExtraField.size());

		assert(bytesCopied == localHeaderSize);
	}
	
	unzCloseCurrentFile(_handle);

	return output;
}

boost::uint32_t ZipFileRead::GetCumulativeCrc()
{
	boost::uint32_t overallCRC = 0;

	int result = unzGoToFirstFile(_handle);

	if (result != UNZ_OK)
	{
		throw std::runtime_error("[GetCumulativeCRC]: Cannot go to first file: " + intToStr(result));
	}

	while (result == UNZ_OK)
	{
		// get zipped file info
		unz_file_info info;
		result = unzGetCurrentFileInfo(_handle, &info, NULL, 0, NULL, 0, NULL, 0);
	
		if (result != UNZ_OK)
		{
			throw std::runtime_error("[GetCumulativeCRC]: Cannot get file info: " + intToStr(result));
		}

		overallCRC ^= info.crc;
		
		result = unzGoToNextFile(_handle);
	}

	return overallCRC;
}

// --------------------------------------------------------

ZipFileWrite::ZipFileWrite(zipFile handle) :
	_handle(handle)
{}

ZipFileWrite::~ZipFileWrite()
{
	zipClose(_handle, _globalComment.empty() ? NULL : _globalComment.c_str());
}

void ZipFileWrite::SetGlobalComment(const std::string& comment)
{
	_globalComment = comment;
}

bool ZipFileWrite::DeflateFile(const fs::path& fileToCompress, const std::string& destPath, CompressionMethod method)
{
	if (!boost::filesystem::exists(fileToCompress))
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[DeflateFile]: Cannot find file for compression " + fileToCompress.file_string());
		return false;
	}

	// Get the current time and date from the given file
	std::time_t changeTime = boost::filesystem::last_write_time(fileToCompress);

	tm* timeinfo = localtime(&changeTime);

	// Create a new info structure
	zip_fileinfo zfi;
	zfi.dosDate = 0;
	zfi.tmz_date.tm_hour = timeinfo->tm_hour;
	zfi.tmz_date.tm_min = timeinfo->tm_min;
	zfi.tmz_date.tm_sec = timeinfo->tm_sec;

	zfi.tmz_date.tm_mday = timeinfo->tm_mday;
	zfi.tmz_date.tm_mon = timeinfo->tm_mon;
	zfi.tmz_date.tm_year = timeinfo->tm_year + 1900;
	zfi.internal_fa = 0;
	zfi.external_fa = 0;

	// Make sure 0-byte files are not DEFLATED, otherwise they end up with 2 bytes compressed size
	if (fs::file_size(fileToCompress) == 0)
	{
		method = STORE;
	}

	// Prepare the zip file for writing
	int status = zipOpenNewFileInZip3(_handle,
					destPath.c_str(),
					&zfi,
					NULL,
					0,
					NULL,
					0,
					NULL,
					(method == DEFLATE || method == DEFLATE_MAX) ? Z_DEFLATED : 0, // deflate or store
					(method == DEFLATE_MAX) ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION,
					0,				// not raw
					MAX_WBITS,		// window bits
					MAX_MEM_LEVEL,	// memory level
					Z_DEFAULT_STRATEGY,	// strategy
					NULL,	// no password
					0);		// no crypting

	if (status != ZIP_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[DeflateFile]: Cannot open file in zip. " + fileToCompress.file_string());
		return false;
	}

	FILE* inFileBinary = fopen(fileToCompress.file_string().c_str(), "rb");

	if (!inFileBinary)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[DeflateFile]: Cannot open destination file. " + destPath);
		zipCloseFileInZip(_handle);
		return false;
	}

	while (1)
	{
		char buf[512*1024];

		size_t bytesRead = fread(buf, 1, sizeof(buf), inFileBinary);

		if (bytesRead > 0)
		{
			status = zipWriteInFileInZip(_handle, buf, bytesRead);

			if (status != ZIP_OK)
			{
				tdm::TraceLog::WriteLine(LOG_VERBOSE, "[DeflateFile]: Failure writing compressed data. " + fileToCompress.file_string() + ": " + intToStr(status));
				zipCloseFileInZip(_handle);
				return false;
			}

			continue;
		}
		
		break;
	}

	status = zipCloseFileInZip(_handle);

	if (status != ZIP_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[DeflateFile]: Failure closing compressed file. " + fileToCompress.file_string() + ": " + intToStr(status));
		return false;
	}

	fclose(inFileBinary);

	return true;
}

bool ZipFileWrite::CopyFileFromZip(const ZipFileReadPtr& fromZip, const std::string& fromPath, const std::string& toPath)
{
	// Get raw data from the other file
	ZipFileRead::CompressedFilePtr file = fromZip->ReadCompressedFile(fromPath);

	if (file == NULL)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[CopyFileFromZip]: Cannot open source zip file " + fromPath);
		return false;
	}

	// Convert the time into zip format
	tm* changeTime = localtime(&file->changeTime);
	
	// open destination file
	zip_fileinfo zfi;
	zfi.dosDate = 0;
	zfi.tmz_date.tm_hour = changeTime->tm_hour;
	zfi.tmz_date.tm_min = changeTime->tm_min;
	zfi.tmz_date.tm_sec = changeTime->tm_sec;

	zfi.tmz_date.tm_mday = changeTime->tm_mday;
	zfi.tmz_date.tm_mon = changeTime->tm_mon;
	zfi.tmz_date.tm_year = changeTime->tm_year + 1900;
	zfi.internal_fa = 0;
	zfi.external_fa = 0;

	void* extraField = !file->extraField.empty() ? &file->extraField.front() : NULL;
	const char* comment = !file->comment.empty() ? &file->comment.front() : NULL;
	void* localExtraField = !file->localExtraField.empty() ? &file->localExtraField.front() : NULL;

	// Carry over compression method, few-byte files like binary.conf are stored
	int method = file->compressionMethod == ZipFileRead::CompressedFile::STORED ? 0 : Z_DEFLATED;
	int level = file->compressionLevel;

	int result = zipOpenNewFileInZip2(_handle, toPath.c_str(), &zfi, 
									  localExtraField, file->localExtraField.size(), 
									  extraField, file->extraField.size(), 
									  comment, method, level, 1);

	if (result != UNZ_OK)
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[CopyFileFromZip]: Cannot open file in zip " + toPath + ": " + intToStr(result));
		return false;
	}

	// Write the raw data
	void* data = !file->data.empty() ? &file->data.front() : NULL;
	result = zipWriteInFileInZip(_handle, data, file->data.size());

	if (result != UNZ_OK) 
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[CopyFileFromZip]: Cannot write file into zip " + toPath + ": " + intToStr(result));
		return false;
	}

	result = zipCloseFileInZipRaw(_handle, file->uncompressedSize, file->crc32);

	if (result != UNZ_OK) 
	{
		tdm::TraceLog::WriteLine(LOG_VERBOSE, "[CopyFileFromZip]: Cannot close file in zip after raw write " + toPath + ": " + intToStr(result));
		return false;
	}

	return true;
}

// --------------------------------------------------------

ZipFileReadPtr Zip::OpenFileRead(const fs::path& fullPath)
{
	unzFile handle = unzOpen(fullPath.file_string().c_str());

	return (handle != NULL) ? ZipFileReadPtr(new ZipFileRead(handle)) : ZipFileReadPtr();
}

ZipFileWritePtr Zip::OpenFileWrite(const fs::path& fullPath, WriteMode mode)
{
	zipFile handle = zipOpen(fullPath.file_string().c_str(), mode == APPEND ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);

	return (handle != NULL) ? ZipFileWritePtr(new ZipFileWrite(handle)) : ZipFileWritePtr();
}

// Local helper class for copying files from one archive into another
class ZipFileCopier :
	public ZipFileRead::Visitor
{
private:
	ZipFileReadPtr _source;
	ZipFileWritePtr _target;
	const std::set<std::string>& _ignoreList;

public:
	ZipFileCopier(const ZipFileReadPtr& source, const ZipFileWritePtr& target, 
		          const std::set<std::string>& ignoreList) :
		_source(source),
		_target(target),
		_ignoreList(ignoreList)
	{}

	void VisitFile(const ZipFileRead::MemberInfo& info)
	{
		if (_ignoreList.find(info.filename) != _ignoreList.end())
		{
			return; // skip this file
		}

		_target->CopyFileFromZip(_source, info.filename, info.filename);
	}
};

void Zip::RemoveFilesFromArchive(const fs::path& fullPath, const std::set<std::string>& membersToRemove)
{
	if (membersToRemove.empty()) return; // quick bail out on empty removal list

	fs::path temporaryPath = fullPath;
	temporaryPath.remove_leaf().remove_leaf();
	temporaryPath /= TMP_FILE_PREFIX + fullPath.leaf();

	TraceLog::WriteLine(LOG_VERBOSE, 
		(boost::format("Removing %d files from archive %s") % membersToRemove.size() % fullPath.file_string()).str());

	{
		ZipFileReadPtr source = OpenFileRead(fullPath);

		if (source == NULL)
		{
			TraceLog::Error("Cannot open archive for reading: " + fullPath.file_string());
			return;
		}

		ZipFileWritePtr target = OpenFileWrite(temporaryPath, CREATE);

		// Copy the global comment of this archive over to the target
		target->SetGlobalComment(source->GetGlobalComment());

		// Instantiate a copier object and walk through the source zip
		ZipFileCopier copier(source, target, membersToRemove);
		source->ForeachFile(copier);
	}

	// Remove the source file
	File::Remove(fullPath);

	// Move the temporary file over the old one
	File::Move(temporaryPath, fullPath);
}

} // namespace
