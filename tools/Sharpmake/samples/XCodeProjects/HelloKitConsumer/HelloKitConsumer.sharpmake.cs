// Copyright (c) 2022-2023 Ubisoft Entertainment
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
using System.IO;

namespace XCodeProjects
{
    [Sharpmake.Generate]
    public class HelloKitConsumerProject : CommonProject
    {
        public HelloKitConsumerProject()
        {
            Name = @"HelloKitConsumer";

            AddTargets(CommonTarget.GetMacTargets());
            AddTargets(CommonTarget.GetIosTargets());
            AddTargets(CommonTarget.GetTvosTargets());
            AddTargets(CommonTarget.GetCatalystTargets());

            SourceRootPath = Util.GetCurrentSharpmakeFileInfo().DirectoryName;
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.Output = Configuration.OutputType.AppleApp;
            conf.AddPublicDependency<FmtProject>(target);
            conf.AddPublicDependency<HelloKitFrameworkProject>(
                target,
                DependencySetting.OnlyBuildOrder
            );

            conf.XcodeFrameworkPaths.Add(Globals.OutputDirectory, @"[conf.TargetPath]");

            conf.XcodeEmbeddedFrameworks.Add(Path.Combine(@"[conf.TargetPath]", "HelloKit.framework"));

            //adding `LD_RUNPATH_SEARCH_PATHS = "@executable_path/Frameworks @rpath/HelloKit.framework/Versions/A/HelloKit";`
            conf.Options.Add(new Options.XCode.Compiler.LdRunPaths(@"@executable_path/Frameworks @rpath/HelloKit.framework/Versions/A/HelloKit"));
        }
    }
}
