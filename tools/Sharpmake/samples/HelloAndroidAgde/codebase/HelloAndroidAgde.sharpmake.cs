﻿// Copyright (c) 2021-2022 Ubisoft Entertainment
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

using System.IO;
using Sharpmake;

namespace HelloAndroidAgde
{
    [Sharpmake.Generate]
    public class HelloAndroidAgdeSolution : CommonSolution
    {
        public HelloAndroidAgdeSolution()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "HelloAndroidAgde";
        }

        private bool _hasCopiedResources = false;
        public override void ConfigureAgde(Configuration conf, CommonTarget target)
        {
            base.ConfigureAgde(conf, target);

            conf.AddProject<ExeProject>(target);

            if (!_hasCopiedResources)
            {
                //copy top-level build gradle files to root dir
                AndroidUtil.DirectoryCopy(Path.Combine(conf.Solution.SharpmakeCsPath, @"..\gradle\root"), conf.SolutionPath);
                _hasCopiedResources = true;

                var gradlePropertiesFile = Path.Combine(conf.SolutionPath, "gradle.properties");
                if (File.Exists(gradlePropertiesFile))
                {
                    using (StreamWriter sw = File.AppendText(gradlePropertiesFile))
                    {
                        sw.WriteLine(string.Format("ndkRoot={0}", Android.GlobalSettings.NdkRoot.Replace("\\", "/")));
                    }
                }
            }
        }
    }
}
