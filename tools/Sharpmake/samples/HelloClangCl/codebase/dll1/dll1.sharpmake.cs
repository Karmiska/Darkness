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

using System;
using System.IO;

namespace HelloClangCl
{
    [Sharpmake.Generate]
    public class Dll1Project : CommonProject
    {
        public Dll1Project()
        {
            AddTargets(CommonTarget.GetDefaultTargets());
            Name = "dll1";
        }

        public override void ConfigureAll(Configuration conf, CommonTarget target)
        {
            base.ConfigureAll(conf, target);

            conf.SolutionFolder = "SharedLibs";

            conf.PrecompHeader = "precomp.h";
            conf.PrecompSource = "precomp.cpp";

            conf.Output = Configuration.OutputType.Dll;

            conf.Defines.Add("UTIL_DLL_EXPORT");
            conf.ExportDefines.Add("UTIL_DLL_IMPORT");

            conf.IncludePaths.Add(SourceRootPath);

            string outputFileName = @"[conf.TargetPath]\[conf.TargetFileFullName].step_output.txt";
            conf.EventPostBuildExecute.Add(
                "sentinel_[conf.Name]",
                new Configuration.BuildStepExecutable(
                    Path.Combine(Environment.SystemDirectory, "cmd.exe"),
                    @"[conf.TargetPath]\[conf.TargetFileFullNameWithExtension]",
                    outputFileName,
                    $@"/c ""echo dll linked > {outputFileName}""",
                    "[conf.TargetPath]"
                )
            );
            conf.EventPostBuild.Add($@"echo dll linked > {outputFileName}");

            conf.AddPrivateDependency<StaticLib1Project>(target);
        }
    }
}
