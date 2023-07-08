﻿// Copyright (c) 2017-2019, 2021-2022 Ubisoft Entertainment
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
using Sharpmake;

namespace PackageReference
{
    [Generate]
    public class CSharpPackageReferences : CSharpProject
    {
        public CSharpPackageReferences()
        {
            AddTargets(
                new Target(
                    Platform.win64,
                    DevEnv.vs2017 | DevEnv.vs2019,
                    Optimization.Debug | Optimization.Release,
                    OutputType.Dll,
                    Blob.NoBlob,
                    BuildSystem.MSBuild,
                    DotNetFramework.v4_7_2
                )
            );

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            // This Path will be used to get all SourceFiles in this Folder and all subFolders
            SourceRootPath = @"[project.SharpmakeCsPath]\codebase\[project.Name]";
            AssemblyName = "PackageReference";
        }

        [Configure()]
        public virtual void ConfigureAll(Configuration conf, Target target)
        {
            conf.ProjectFileName = "[project.Name].[target.DevEnv].[target.Framework]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.TargetPath = @"[conf.ProjectPath]\output\[target.DevEnv]\[conf.Name]";

            conf.Options.Add(Options.CSharp.TreatWarningsAsErrors.Enabled);

            conf.ReferencesByNuGetPackage.Add("NUnit", "3.6.0");
            conf.ReferencesByNuGetPackage.Add("Newtonsoft.Json", "13.0.1");
            conf.ReferencesByNuGetPackage.Add("Mono.Cecil", "0.9.6.4", privateAssets: Sharpmake.PackageReferences.AssetsDependency.All);
            conf.ReferencesByNuGetPackage.Add("MySql.Data", "6.10.6", privateAssets: Sharpmake.PackageReferences.AssetsDependency.Build | Sharpmake.PackageReferences.AssetsDependency.Compile);
        }
    }

    [Generate]
    public class CPPPackageReferences : Project
    {
        public CPPPackageReferences()
        {
            AddTargets(
                new Target(
                    Platform.win64,
                    DevEnv.vs2017 | DevEnv.vs2019,
                    Optimization.Debug | Optimization.Release,
                    OutputType.Dll,
                    Blob.NoBlob,
                    BuildSystem.MSBuild,
                    DotNetFramework.v4_7_2
                )
            );

            RootPath = @"[project.SharpmakeCsPath]\projects\[project.Name]";

            // This Path will be used to get all SourceFiles in this Folder and all subFolders
            SourceRootPath = @"[project.SharpmakeCsPath]\codebase\[project.Name]";
        }

        [Configure()]
        public virtual void ConfigureAll(Configuration conf, Target target)
        {
            conf.ProjectFileName = "[project.Name].[target.DevEnv].[target.Framework]";
            conf.ProjectPath = @"[project.RootPath]";

            conf.TargetPath = @"[conf.ProjectPath]\output\[target.DevEnv]\[conf.Name]";

            conf.Options.Add(Options.Vc.Compiler.Exceptions.Enable);

            conf.ReferencesByNuGetPackage.Add("gtest-vc140-static-64", "1.1.0");
        }
    }

    [Generate]
    public class PackageReferenceSolution : CSharpSolution
    {
        public PackageReferenceSolution()
        {
            AddTargets(
                new Target(
                    Platform.win64,
                    DevEnv.vs2017 | DevEnv.vs2019,
                    Optimization.Debug | Optimization.Release,
                    OutputType.Dll,
                    Blob.NoBlob,
                    BuildSystem.MSBuild,
                    DotNetFramework.v4_7_2
                )
            );
        }

        [Configure()]
        public void ConfigureAll(Configuration conf, Target target)
        {
            conf.SolutionFileName = string.Format("{0}.{1}.{2}",
                                                  Name,
                                                  "[target.DevEnv]",
                                                  "[target.Framework]");
            conf.SolutionPath = @"[solution.SharpmakeCsPath]\projects\";

            conf.AddProject<CSharpPackageReferences>(target);
            conf.AddProject<CPPPackageReferences>(target);
        }

        [Main]
        public static void SharpmakeMain(Arguments arguments)
        {
            KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2017, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.v10_0_17763_0);
            KitsRootPaths.SetUseKitsRootForDevEnv(DevEnv.vs2019, KitsRootEnum.KitsRoot10, Options.Vc.General.WindowsTargetPlatformVersion.v10_0_19041_0);
            arguments.Generate<PackageReferenceSolution>();
        }
    }
}
