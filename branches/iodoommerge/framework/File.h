/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5243 $ (Revision of last commit) 
 $Date: 2012-02-03 09:28:06 -0500 (Fri, 03 Feb 2012) $ (Date of last commit)
 $Author: tels $ (Author of last commit)
 
******************************************************************************/

#ifndef __FILE_H__
#define __FILE_H__

#define	MAX_PRINT_MSG_SIZE	16 * 1024

/*
==============================================================

  File Streams.

==============================================================
*/

// mode parm for Seek
typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

class idFileSystemLocal;


class idFile {
public:
	virtual					~idFile( void ) {};
							// Get the name of the file.
	virtual const char *	GetName( void );
							// Get the full file path.
	virtual const char *	GetFullPath( void );
							// Read data from the file to the buffer.
	virtual int				Read( void *buffer, int len );
							// Write data from the buffer to the file.
	virtual int				Write( const void *buffer, int len );
							// Returns the length of the file.
	virtual int				Length( void );
							// Return a time value for reload operations.
	virtual ID_TIME_T			Timestamp( void );
							// Returns offset in file.
	virtual int				Tell( void );
							// Forces flush on files being writting to.
	virtual void			ForceFlush( void );
							// Causes any buffered data to be written to the file.
	virtual void			Flush( void );
							// Seek on a file.
	virtual int				Seek( long offset, fsOrigin_t origin );
							// Go back to the beginning of the file.
	virtual void			Rewind( void );
							// Like fprintf.
	virtual int				Printf( const char *fmt, ... ) id_attribute((format(printf,2,3)));
							// Like fprintf but with argument pointer
	virtual int				VPrintf( const char *fmt, va_list arg );
							// Write a string with high precision floating point numbers to the file.
	virtual int				WriteFloatString( const char *fmt, ... ) id_attribute((format(printf,2,3)));
	
	// Endian portable alternatives to Read(...)
	virtual int				ReadInt( int &value );
	virtual int				ReadUnsignedInt( unsigned int &value );
	virtual int				ReadShort( short &value );
	virtual int				ReadUnsignedShort( unsigned short &value );
	virtual int				ReadChar( char &value );
	virtual int				ReadUnsignedChar( unsigned char &value );
	virtual int				ReadFloat( float &value );
	virtual int				ReadBool( bool &value );
	virtual int				ReadString( idStr &string );
	virtual int				ReadVec2( idVec2 &vec );
	virtual int				ReadVec3( idVec3 &vec );
	virtual int				ReadVec4( idVec4 &vec );
	virtual int				ReadVec6( idVec6 &vec );
	virtual int				ReadMat3( idMat3 &mat );
	
	// Endian portable alternatives to Write(...)
	virtual int				WriteInt( const int value );
	virtual int				WriteUnsignedInt( const unsigned int value );
	virtual int				WriteShort( const short value );
	virtual int				WriteUnsignedShort( unsigned short value );
	virtual int				WriteChar( const char value );
	virtual int				WriteUnsignedChar( const unsigned char value );
	virtual int				WriteFloat( const float value );
	virtual int				WriteBool( const bool value );
	virtual int				WriteString( const char *string );
	virtual int				WriteVec2( const idVec2 &vec );
	virtual int				WriteVec3( const idVec3 &vec );
	virtual int				WriteVec4( const idVec4 &vec );
	virtual int				WriteVec6( const idVec6 &vec );
	virtual int				WriteMat3( const idMat3 &mat );
};


class idFile_Memory : public idFile {
	friend class			idFileSystemLocal;

public:
							idFile_Memory( void );	// file for writing without name
							idFile_Memory( const char *name );	// file for writing
							idFile_Memory( const char *name, char *data, int length );	// file for writing
							idFile_Memory( const char *name, const char *data, int length );	// file for reading
	virtual					~idFile_Memory( void );

	virtual const char *	GetName( void ) { return name.c_str(); }
	virtual const char *	GetFullPath( void ) { return name.c_str(); }
	virtual int				Read( void *buffer, int len );
	virtual int				Write( const void *buffer, int len );
	virtual int				Length( void );
	virtual ID_TIME_T			Timestamp( void );
	virtual int				Tell( void );
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual int				Seek( long offset, fsOrigin_t origin );

							// changes memory file to read only
	virtual void			MakeReadOnly( void );
							// clear the file
	virtual void			Clear( bool freeMemory = true );
							// set data for reading
	void					SetData( const char *data, int length );
							// returns const pointer to the memory buffer
	const char *			GetDataPtr( void ) const { return filePtr; }
							// set the file granularity
	void					SetGranularity( int g ) { assert( g > 0 ); granularity = g; }

private:
	idStr					name;			// name of the file
	int						mode;			// open mode
	int						maxSize;		// maximum size of file
	int						fileSize;		// size of the file
	int						allocated;		// allocated size
	int						granularity;	// file granularity
	char *					filePtr;		// buffer holding the file data
	char *					curPtr;			// current read/write pointer
};


class idFile_BitMsg : public idFile {
	friend class			idFileSystemLocal;

public:
							idFile_BitMsg( idBitMsg &msg );
							idFile_BitMsg( const idBitMsg &msg );
	virtual					~idFile_BitMsg( void );

	virtual const char *	GetName( void ) { return name.c_str(); }
	virtual const char *	GetFullPath( void ) { return name.c_str(); }
	virtual int				Read( void *buffer, int len );
	virtual int				Write( const void *buffer, int len );
	virtual int				Length( void );
	virtual ID_TIME_T			Timestamp( void );
	virtual int				Tell( void );
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual int				Seek( long offset, fsOrigin_t origin );

private:
	idStr					name;			// name of the file
	int						mode;			// open mode
	idBitMsg *				msg;
};


class idFile_Permanent : public idFile {
	friend class			idFileSystemLocal;

public:
							idFile_Permanent( void );
	virtual					~idFile_Permanent( void );

	virtual const char *	GetName( void ) { return name.c_str(); }
	virtual const char *	GetFullPath( void ) { return fullPath.c_str(); }
	virtual int				Read( void *buffer, int len );
	virtual int				Write( const void *buffer, int len );
	virtual int				Length( void );
	virtual ID_TIME_T			Timestamp( void );
	virtual int				Tell( void );
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual int				Seek( long offset, fsOrigin_t origin );

	// returns file pointer
	FILE *					GetFilePtr( void ) { return o; }

private:
	idStr					name;			// relative path of the file - relative path
	idStr					fullPath;		// full file path - OS path
	int						mode;			// open mode
	int						fileSize;		// size of the file
	FILE *					o;				// file handle
	bool					handleSync;		// true if written data is immediately flushed
};


class idFile_InZip : public idFile {
	friend class			idFileSystemLocal;

public:
							idFile_InZip( void );
	virtual					~idFile_InZip( void );

	virtual const char *	GetName( void ) { return name.c_str(); }
	virtual const char *	GetFullPath( void ) { return fullPath.c_str(); }
	virtual int				Read( void *buffer, int len );
	virtual int				Write( const void *buffer, int len );
	virtual int				Length( void );
	virtual ID_TIME_T			Timestamp( void );
	virtual int				Tell( void );
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual int				Seek( long offset, fsOrigin_t origin );

private:
	idStr					name;			// name of the file in the pak
	idStr					fullPath;		// full file path including pak file name
	int						zipFilePos;		// zip file info position in pak
	int						fileSize;		// size of the file
	void *					z;				// unzip info
};

#endif /* !__FILE_H__ */
