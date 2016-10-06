/*
 * Copyright 2016 (c). All rights reserved.
 * Author: Paul Glendenning
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 *    * Neither the name of the author, nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <lucida/path_ops.h>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#include <iostream>

using namespace boost::filesystem;

namespace {
#ifdef _MSC_VER
#define	URLFILE_OFFS	8
static const std::string fileURLPrefix("file:///") ;
static const std::string rootPrefix("/");
#else
#define	URLFILE_OFFS	7
static const std::string fileURLPrefix("file://");
static const std::string rootPrefix; 
#endif

// Boost 1.5 does not have this function - why?
#if (BOOST_VERSION/100000) == 1 && (BOOST_VERSION/100 % 1000) < 60
::boost::filesystem::path relative(const ::boost::filesystem::path &abspath, const ::boost::filesystem::path &relative_to) {
	using namespace boost::filesystem;
	// create absolute paths
	path p = absolute(abspath);
	path r = absolute(relative_to);

	// if root paths are different, return absolute path
	if (p.root_path() != r.root_path())
		return p;

	// initialize relative path
	path result;

	// find out where the two paths diverge
	path::const_iterator itr_path = p.begin();
	path::const_iterator itr_relative_to = r.begin();
	while (*itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end()) {
		++itr_path;
		++itr_relative_to;
	}

	// add "../" for each remaining token in relative_to
	if (itr_relative_to != r.end()) {
		++itr_relative_to;
		while(itr_relative_to != r.end()) {
			result /= "..";
			++itr_relative_to;
		}
	}

	// add remaining path
	while (itr_path != p.end()) {
		result /= *itr_path;
		++itr_path;
	}
	return result;
}
#endif
} // anonymous namespace

namespace lucida {

std::string Basename(const std::string& pathname) {
	const char* pbase = pathname.c_str();
	const char* p     = pbase + pathname.size();
#ifdef  _MSC_VER
	while (--p >= pbase && *p != '/' && *p != '\\')
		/* continue */;
#else
	while (--p >= pbase && *p != '/')
		/* continue */;
#endif
	return p+1;
}


std::string Dirname(const std::string& pathname, unsigned level)
{
	const char* pbase = pathname.c_str();
	const char* p     = pbase + pathname.size();
#ifdef  _MSC_VER
	while (--p >= pbase && *p != '/' && *p != '\\')
		/* continue */;
#else
	while (--p >= pbase && *p != '/')
		/* continue */;
#endif
	return pathname.substr(0, p-pbase);;
}

bool MakeAbsolutePathOrUrl(std::string& abspath, const std::string& relpath, const std::string& workdir) {
	size_t n  = workdir.find("://");
	std::string protocol;

	abspath.clear();
	if (relpath.find("://") != std::string::npos)
		return false;

	path pabs, prel(relpath);
	if (prel.has_root_directory() || !prel.is_relative())
		return false; 

	if (n != std::string::npos) {
		if (0 == workdir.find("file:///")) {
			protocol = workdir.substr(0, URLFILE_OFFS);
			pabs = path(workdir.substr(URLFILE_OFFS));
		} else {
			// URL protocol != file
			protocol = workdir.substr(0, n);
			pabs = path(workdir.substr(n));
		}
	} else pabs = path(workdir);

	pabs = absolute(prel, pabs);
	if (pabs.has_root_directory()) {
		abspath = protocol + pabs.normalize().generic_string();
		return true;
	} else if (pabs.is_relative() && protocol.empty()) {
		abspath = pabs.normalize().generic_string();
		return true;
	}
	return false;
}


bool MakeAbsolutePath(std::string& abspath, const std::string& relpath) {
	return MakeAbsolutePathOrUrl(abspath, relpath, current_path().string());
}


bool MakeRelativePathOrUrl(std::string& relpath, const std::string& abspath, const std::string& workdir) { 
	size_t n  = workdir.find("://");

	relpath.clear();
	if (abspath.find("://") != std::string::npos)
		return false;

	path pabs, pdir;
	if (n != std::string::npos) {
		if (0 == workdir.find("file:///")) {
			n = URLFILE_OFFS;
		}
		std::string protocol = workdir.substr(0, n);
		if (abspath.find(protocol) == std::string::npos)
			return false;
		pdir = path(workdir.substr(n));
		pabs = path(abspath.substr(n));
	} else {
		pdir = path(workdir);
		pabs = path(abspath);
	}
	
	if (!pabs.has_root_directory() || !pdir.has_root_directory())
		return false;

	if (pabs.normalize().generic_string().find(pdir.normalize().generic_string()) == std::string::npos)
		return false; // different root

	path prel = relative(pabs, pdir);
	relpath = prel.normalize().generic_string();
	return true;
}


bool MakeRelativePath(std::string& relpath, const std::string& abspath, const std::string& workdir) { 
	return MakeRelativePathOrUrl(relpath, abspath, current_path().string());
}


} // namespace lucida
