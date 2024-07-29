cd bin
./judoboard_test --gtest_output=xml --gtest_shuffle --gtest_filter=-Mat.*:RemoteMat.*
if [ $? -ne 0 ]
then
  echo "Unit tests failed"
  cd ..
  exit 1
fi
./judoboard_test --gtest_output=xml --gtest_shuffle --gtest_filter=Mat.*
if [ $? -ne 0 ]
then
  echo "Unit tests failed"
  cd ..
  exit 1
fi
./judoboard_test --gtest_output=xml --gtest_shuffle --gtest_filter=RemoteMat.*
if [ $? -ne 0 ]
then
  echo "Unit tests failed"
  cd ..
  exit 1
fi


echo "All test ran successfull"
cd ..
exit 0