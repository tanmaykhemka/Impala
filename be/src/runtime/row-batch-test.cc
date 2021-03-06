// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "testutil/death-test-util.h"
#include "testutil/gtest-util.h"
#include "runtime/mem-tracker.h"
#include "runtime/row-batch.h"
#include "testutil/desc-tbl-builder.h"

#include <gtest/gtest.h>

#include "common/names.h"

namespace impala {

TEST(RowBatchTest, AcquireStateWithMarkAtCapacity) {
  // Test that AcquireState() can be correctly called with MarkAtCapacity() on the
  // source batch.
  ObjectPool pool;
  DescriptorTblBuilder builder(&pool);
  builder.DeclareTuple() << TYPE_INT;
  DescriptorTbl* desc_tbl = builder.Build();

  vector<bool> nullable_tuples = {false};
  vector<TTupleId> tuple_id = {static_cast<TupleId>(0)};
  RowDescriptor row_desc(*desc_tbl, tuple_id, nullable_tuples);
  MemTracker tracker;
  {
    RowBatch src(row_desc, 1024, &tracker);
    src.AddRow(); src.CommitLastRow();
    // Calls MarkAtCapacity().
    src.MarkNeedToReturn();

    // Note InitialCapacity(), not capacity(). Latter will DCHECK.
    RowBatch dest(row_desc, src.InitialCapacity(), &tracker);
    dest.AcquireState(&src);
  }

  // Confirm the bad pattern causes an error.
  {
    RowBatch src(row_desc, 1024, &tracker);
    src.AddRow(); src.CommitLastRow();
    // Calls MarkAtCapacity().
    src.MarkNeedToReturn();
    RowBatch bad_dest(row_desc, src.capacity(), &tracker);
    IMPALA_ASSERT_DEBUG_DEATH(bad_dest.AcquireState(&src), "");
  }
}

}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
