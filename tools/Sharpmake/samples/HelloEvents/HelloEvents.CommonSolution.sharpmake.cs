﻿// Copyright (c) 2021 Ubisoft Entertainment
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using Sharpmake;

namespace HelloEvents
{
    public class CommonSolution : Sharpmake.Solution
    {
        public CommonSolution()
            : base(typeof(CommonTarget))
        {
            IsFileNameToLower = false;
        }

        [ConfigurePriority(ConfigurePriorities.All)]
        [Configure]
        public virtual void ConfigureAll(Configuration conf, CommonTarget target)
        {
            conf.SolutionFileName = "[solution.Name]_[target.Platform]";
            if (target.DevEnv != DevEnv.xcode)
                conf.SolutionFileName += "_[target.DevEnv]";
            if (target.BuildSystem == BuildSystem.FastBuild)
                conf.Name += "_FastBuild";
            conf.PlatformName = "[target.SolutionPlatformName]";
            conf.SolutionPath = System.IO.Path.Combine(Globals.TmpDirectory, "solutions");
        }
    }
}
