/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5122 $ (Revision of last commit) 
 $Date: 2011-12-11 14:47:31 -0500 (Sun, 11 Dec 2011) $ (Date of last commit)
 $Author: greebo $ (Author of last commit)
 
******************************************************************************/

#ifndef __DECLPDA_H__
#define __DECLPDA_H__

/*
===============================================================================

	idDeclPDA

===============================================================================
*/


class idDeclEmail : public idDecl {
public:
							idDeclEmail() {}

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void ) const;
	virtual void			List( void ) const;

	const char *			GetFrom() const { return from; }
	const char *			GetBody() const { return text; }
	const char *			GetSubject() const { return subject; }
	const char *			GetDate() const { return date; }
	const char *			GetTo() const { return to; }
	const char *			GetImage() const { return image; }

private:
	idStr					text;
	idStr					subject;
	idStr					date;
	idStr					to;
	idStr					from;
	idStr					image;
};


class idDeclVideo : public idDecl {
public:
							idDeclVideo() {};

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void ) const;
	virtual void			List( void ) const;

	const char *			GetRoq() const { return video; }
	const char *			GetWave() const { return audio; }
	const char *			GetVideoName() const { return videoName; }
	const char *			GetInfo() const { return info; }
	const char *			GetPreview() const { return preview; }

private:
	idStr					preview;
	idStr					video;
	idStr					videoName;
	idStr					info;
	idStr					audio;
};


class idDeclAudio : public idDecl {
public:
							idDeclAudio() {};

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void ) const;
	virtual void			List( void ) const;

	const char *			GetAudioName() const { return audioName; }
	const char *			GetWave() const { return audio; }
	const char *			GetInfo() const { return info; }
	const char *			GetPreview() const { return preview; }

private:
	idStr					audio;
	idStr					audioName;
	idStr					info;
	idStr					preview;
};


class idDeclPDA : public idDecl {
public:
							idDeclPDA() { originalEmails = originalVideos = 0; };

	virtual size_t			Size( void ) const;
	virtual const char *	DefaultDefinition( void ) const;
	virtual bool			Parse( const char *text, const int textLength );
	virtual void			FreeData( void );
	virtual void			Print( void ) const;
	virtual void			List( void ) const;

	virtual void			AddVideo( const char *name, bool unique = true ) const;
	virtual void			AddAudio( const char *name, bool unique = true ) const;
	virtual void			AddEmail( const char *name, bool unique = true ) const;
	virtual void			RemoveAddedEmailsAndVideos() const;

	virtual const int		GetNumVideos() const;
	virtual const int		GetNumAudios() const;
	virtual const int		GetNumEmails() const;
	virtual const idDeclVideo *GetVideoByIndex( int index ) const;
	virtual const idDeclAudio *GetAudioByIndex( int index ) const;
	virtual const idDeclEmail *GetEmailByIndex( int index ) const;

	virtual void			SetSecurity( const char *sec ) const;

	const char *			GetPdaName() const { return pdaName; }
	const char *			GetSecurity() const {return security; }
	const char *			GetFullName() const { return fullName; }
	const char *			GetIcon() const { return icon; }
	const char *			GetPost() const { return post; }
	const char *			GetID() const { return id; }
	const char *			GetTitle() const { return title; }

private:
	mutable idStrList		videos;
	mutable idStrList		audios;
	mutable idStrList		emails;
	idStr					pdaName;
	idStr					fullName;
	idStr					icon;
	idStr					id;
	idStr					post;
	idStr					title;
	mutable idStr			security;
	mutable	int				originalEmails;
	mutable int				originalVideos;
};

#endif /* !__DECLPDA_H__ */
