cd bin
echo "Running tests with gdb attached"
gdb -ex=r --args ./judoboard_test_debug --gtest_output=xml
if [ $? -eq 0 ]
then
  echo "All test ran successfull"
  cd ..
  exit 0
else
  echo "Unit tests failed"
  cd ..
  exit 1
fi