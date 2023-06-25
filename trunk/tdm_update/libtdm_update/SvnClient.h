/***************************************************************************
 *
 * PROJECT: The Dark Mod - Updater
 * $Revision: 4439 $
 * $Date: 2011-01-17 01:32:16 -0500 (Mon, 17 Jan 2011) $
 * $Author: greebo $
 *
 ***************************************************************************/

#pragma once

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

// Forward decl.
typedef struct apr_pool_t apr_pool_t;
typedef struct svn_client_ctx_t svn_client_ctx_t;

namespace tdm
{

/**
 * An object providing a few SVN client methods.
 * This is used to query file states.
 */
class SvnClient
{
private:
	// SVN memory pool
	apr_pool_t* _pool;

	// The SVN client context
	svn_client_ctx_t* _context;

	bool _isActive;

public:
	SvnClient();

	~SvnClient();

	// Activate/Deacticate the client. Deactivated clients will return true in FileIsUnderVersionControl().
	void SetActive(bool active);

	// Returns true if the given file is under version control, false in all other cases
	bool FileIsUnderVersionControl(const fs::path& path);
};

}
