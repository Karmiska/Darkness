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

namespace FastBuild
{
    [Sharpmake.Generate]
    public class CustomBuildStepProject : Project
    {
        public CustomBuildStepProject()
        {
            Name = "CustomBuildStep";
            SourceRootPath = @"[project.SharpmakeCsPath]\codebase";
            SourceFilesExtensions.Add(".bat");

            // need to add it explicitly since it's gonna be generated it doesn't exist yet
            SourceFiles.Add(@"[project.SourceRootPath]\main.cpp");

            AddTargets(
                new Target(
                    Platform.win64,
                    DevEnv.vs2017,
                    Optimization.Debug | Optimization.Release,
                    OutputType.Lib,
                    Blob.NoBlob,
                    BuildSystem.MSBuild | BuildSystem.FastBuild
                )
            );
        }

        [Configure]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.CustomFileBuildSteps.Add(
                new Configuration.CustomFileBuildStep
                {
                    KeyInput = "filegeneration.bat",
                    Output = "main.cpp",
                    Description = $"Generate main.cpp",
                    Executable = "filegeneration.bat"
                }
            );

            conf.ProjectFileName = "[project.Name]_[target.DevEnv]_[target.Platform]";
            conf.ProjectPath = @"[project.SharpmakeCsPath]\projects";
        }

        [Configure(BuildSystem.FastBuild)]
        public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.Name = "FastBuild " + conf.Name;
            conf.IsFastBuild = true;
            conf.FastBuildBlobbed = target.Blob == Blob.FastBuildUnitys;
        }
    }

    [Sharpmake.Generate]
    public class CustomBuildStepSolution : Sharpmake.Solution
    {
        public CustomBuildStepSolution()
        {
            Name = "CustomBuildStepSolution";

            AddTargets(
                new Target(
                    Platform.win64,
                    DevEnv.vs2017,
                    Optimization.Debug | Optimization.Release,
                    OutputType.Lib,
                    Blob.NoBlob,
                    BuildSystem.MSBuild | BuildSystem.FastBuild
                )
            );
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.SolutionFileName = "[solution.Name]_[target.DevEnv]_[target.Platform]";
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects";

            conf.AddProject<CustomBuildStepProject>(target);
        }

        [Configure(BuildSystem.FastBuild)]
        public void ConfigureFastBuild(Configuration conf, Target target)
        {
            conf.Name = "FastBuild " + conf.Name;
        }

        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            FastBuildSettings.FastBuildMakeCommand = @"..\..\..\tools\FastBuild\Windows-x64\FBuild.exe";
            KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2017, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.v10_0_17763_0);

            arguments.Generate<CustomBuildStepSolution>();
        }
    }
}
