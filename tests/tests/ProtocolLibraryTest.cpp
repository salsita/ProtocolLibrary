#include "stdafx.h"
#include "ProtocolLibrary_i.h"
#include "ProtocolLibrary_i.c"

//============================================================================
// Fixture
//============================================================================

class ProtocolLibraryTest :
  public ::testing::Test
{
};

//============================================================================
// TESTS
//============================================================================

//----------------------------------------------------------------------------
// NULL pointer checks
TEST_F(ProtocolLibraryTest, CreateInstance)
{
  HRESULT hr = S_OK;
  ASSERT_EQ(S_OK, hr);
}

