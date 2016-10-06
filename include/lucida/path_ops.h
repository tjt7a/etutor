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
#ifndef PATH_H_21E35A1F_AB5A_4403_B107_114CF11FACCA
#define PATH_H_21E35A1F_AB5A_4403_B107_114CF11FACCA
#include <string>

namespace lucida {

/// Make an absolute path, or URL, from a relative path and working directory.
/// If workdir is a URL then relpath must not contain the protocol prefix.
///
///
/// @param[out] abspath     An absolute path or URL.
/// @param[in]  relpath     A relative path or URL.
/// @param[in]  workdir     The working directory.
/// @return True if successful. 
/// @see MakeRelativePathOrUrl()
/// @see MakeAbsolutePath()
bool MakeAbsolutePathOrUrl(std::string& abspath, const std::string& relpath, const std::string& workdir);

/// Make an absolute path from a relative path and the current working directory.
///
/// @param[out] abspath     An absolute path or URL.
/// @param[in]  relpath     A relative path.
/// @return True if successful. 
bool MakeAbsolutePath(std::string& abspath, const std::string& relpath);

/// Make a relative path, or URL, from an absolute path and working directory.
/// If abspath or workdir are URL's then both most have the same protocol
/// prefix.
///
/// @param[out] relpath     A relative path or URL.
/// @param[in]  abspath     An absolute path or URL.
/// @param[in]  workdir     The working directory.
/// @return True if successful. 
/// @see MakeAbsolutePathOrUrl()
bool MakeRelativePathOrUrl(std::string& relpath, const std::string& abspath, const std::string& workdir);

/// Make a relative path from an absolute path and the current working directory.
///
/// @param[out] relpath     A relative path or URL.
/// @param[in]  abspath     An absolute path or URL.
/// @return True if successful. 
bool MakeRelativePath(std::string& relpath, const std::string& abspath);

/// Get the file component of a path.
///
/// @param[in]  pathname    The file pathname.
/// @return     The basename of the path.
std::string Basename(const std::string& pathname);

/// Get the directory component of a path.
///
/// @param[in]  pathname    The file pathname.
/// @param[in]  level       The number of directory levels to traverse.
/// @return     The directory of the path.
std::string Dirname(const std::string& pathname, unsigned level=1);

}       // namespace lucida
#endif  // PATH_H_21E35A1F_AB5A_4403_B107_114CF11FACCA
