/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4476 $
 * $Date: 2011-01-26 03:03:02 -0500 (Wed, 26 Jan 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include "SvnClient.h"

#ifdef USE_LIBSVN

// Forward decl.
typedef struct apr_pool_t apr_pool_t;
typedef struct svn_client_ctx_t svn_client_ctx_t;

namespace tdm
{

// Implementation
class SvnClientImpl :
	public SvnClient
{
private:
	// SVN memory pool
	apr_pool_t* _pool;

	// The SVN client context
	svn_client_ctx_t* _context;

	bool _isActive;

public:
	SvnClientImpl();

	virtual ~SvnClientImpl();

	// Activate/Deacticate the client. Deactivated clients will return true in FileIsUnderVersionControl().
	virtual void SetActive(bool active);

	// Returns true if the given file is under version control, false in all other cases
	virtual bool FileIsUnderVersionControl(const fs::path& path);
};

} // namespace

#endif
