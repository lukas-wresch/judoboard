cd bin
echo "Running tests with gdb attached"
gdb -ex=r -ex=bt -ex="set confirm off" -ex=q --args ./judoboard_test_debug --gtest_output=xml
if [ $? -ne 0 ]
then
  echo "All test ran successfull"
  cd ..
  exit 0
else
  echo "Unit tests failed"
  cd ..
  exit 1
fi