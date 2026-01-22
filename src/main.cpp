#include <iostream>
#include <limits>
#include <vector>

int jump(std::vector<int>& nums) {
  std::vector<std::size_t> dp(nums.size(), std::numeric_limits<std::size_t>::max());
  dp.front() = 0;
  for (std::size_t i = 0; i < nums.size() - 1; ++i) {
    const std::size_t jumpLength = nums[i];
    const auto prevSteps = dp[i];
    if (prevSteps == std::numeric_limits<std::size_t>::max())
    {
      continue;
    }

    for (std::size_t j = 1; j <= jumpLength && i + j < nums.size(); ++j) {
      dp[j + i] = std::min(dp[j], prevSteps + 1);
    }
  }

  return dp.back();
}

int main() {
  std::vector a{2,3,1,1,4};
  jump(a);
}
