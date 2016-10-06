#include <sstream>
#include <lucida/path_ops.h>
#include <gtest/gtest.h>
#include <glog/logging.h>


using namespace lucida;
namespace lucida { namespace test {


TEST(PathTest, GenericPaths) {
	std::string result;
	EXPECT_EQ(Basename("/a/path/to/file.ext"), "file.ext");
	EXPECT_EQ(Dirname("/a/path/to/file.ext"), "/a/path/to");
	EXPECT_TRUE(MakeAbsolutePathOrUrl(result, "adir", "/a/workdir"));
	EXPECT_EQ(result, "/a/workdir/adir");
	EXPECT_TRUE(MakeRelativePathOrUrl(result, "/a/workdir/adir", "/a/workdir"));
	EXPECT_EQ(result, "adir");

	EXPECT_TRUE(MakeAbsolutePathOrUrl(result, "path/to/file.ext", "."));
	EXPECT_EQ(result, "./path/to/file.ext");
	EXPECT_TRUE(MakeAbsolutePathOrUrl(result, "./path/to/file.ext", "."));
	EXPECT_EQ(result, "./path/to/file.ext");
	EXPECT_TRUE(MakeAbsolutePathOrUrl(result, "path/to/file.ext", "./"));
	EXPECT_EQ(result, "./path/to/file.ext");
	EXPECT_TRUE(MakeAbsolutePathOrUrl(result, "./path/to/file.ext", "./"));
	EXPECT_EQ(result, "./path/to/file.ext");
}

} } // lucida::test
