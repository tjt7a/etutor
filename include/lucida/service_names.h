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
#ifndef SERVICE_NAMES_H_CD45B40B_85AC_43B7_B080_BE7ABF5BAE5A
#define SERVICE_NAMES_H_CD45B40B_85AC_43B7_B080_BE7ABF5BAE5A

#include <string>

namespace lucida {

/// Lucida service names and type names.
class ServiceNames {
private:
	ServiceNames();
	ServiceNames(const ServiceNames&);
	ServiceNames& operator = (const ServiceNames&);

public:
	static const char* learnCommandName;
	static const char* createCommandName;
	static const char* inferCommandName;
	static const char* textTypeName;
	static const char* urlTypeName;
	static const char* imageTypeName;
	static const char* unlearnTypeName;

	/// Check validity of a type name.
	///
	/// @param  typeName The type name to test.
	/// @return True if typeName is a valid type.
	static bool isTypeName(const std::string& typeName) {
		return typeName == imageTypeName || typeName == urlTypeName || typeName == textTypeName ||
			typeName == unlearnTypeName;
	}

	/// Check validity of a command name.
	/// 
	/// @param  cmdName  The command name to test.
	/// @return True if cmdName is a valid command.
	static bool isCommandName(const std::string& cmdName) {
		return cmdName == learnCommandName || cmdName == createCommandName || cmdName == inferCommandName;
	}
};
}       // namespace lucida
#endif  // SERVICE_NAMES_H_CD45B40B_85AC_43B7_B080_BE7ABF5BAE5A
