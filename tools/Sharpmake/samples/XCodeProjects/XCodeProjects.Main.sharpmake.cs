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

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Sharpmake;
using Sharpmake.Generators.FastBuild;

[module: Sharpmake.Include("XCodeProjects.*.sharpmake.cs")]
[module: Sharpmake.Include("*/*.sharpmake.cs")]
[module: Sharpmake.Include("extern/*/*.sharpmake.cs")]

namespace XCodeProjects
{
    public static class Globals
    {
        // branch root path relative to current sharpmake file location
        public static readonly string RelativeRootPath = @"./";
        public static string RootDirectory
        {
            get =>
                Path.GetFullPath(
                    Path.Combine(
                        Util.GetCurrentSharpmakeFileInfo().DirectoryName,
                        Globals.RelativeRootPath
                    )
                );
        }

        public static string TmpDirectory
        {
            get => Path.Combine(RootDirectory, "temp");
        }

        public static string OutputDirectory
        {
            get => Path.Combine(TmpDirectory, "bin");
        }

        public static string ExternalDirectory
        {
            get => Path.Combine(RootDirectory, @"..\external");
        }

        public static string[] IncludesDirectories
        {
            get =>
                new string[]
                {
                    Path.Combine(RootDirectory, "extern", "includes"),
                    Path.Combine(RootDirectory, "intern", "includes"),
                };
        }

        public static string LibrariesDirectory
        {
            get => Path.Combine(RootDirectory, "extern", "libs");
        }

        public static string FrameworksDirectory
        {
            get => Path.Combine(RootDirectory, "extern", "frameworks");
        }
    }

    public static class Main
    {
        [Sharpmake.Main]
        public static void SharpmakeMain(Sharpmake.Arguments arguments)
        {
            ConfigureAutoCleanup();

            FastBuildSettings.FastBuildWait = true;
            FastBuildSettings.FastBuildSummary = false;
            FastBuildSettings.FastBuildNoSummaryOnError = true;
            FastBuildSettings.FastBuildDistribution = false;
            FastBuildSettings.FastBuildMonitor = true;
            FastBuildSettings.FastBuildAllowDBMigration = true;

            // for the purpose of this sample, we'll reuse the FastBuild executables that live in the sharpmake source repo
            string sharpmakeFastBuildDir = Util.PathGetAbsolute(
                Globals.RootDirectory,
                @"../../tools/FastBuild"
            );
            switch (Util.GetExecutingPlatform())
            {
                case Platform.linux:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(
                        sharpmakeFastBuildDir,
                        "Linux-x64",
                        "fbuild"
                    );
                    break;
                case Platform.mac:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(
                        sharpmakeFastBuildDir,
                        "OSX-x64",
                        "FBuild"
                    );
                    break;
                case Platform.win64:
                default:
                    FastBuildSettings.FastBuildMakeCommand = Path.Combine(
                        sharpmakeFastBuildDir,
                        "Windows-x64",
                        "FBuild.exe"
                    );
                    break;
            }

            Bff.UnityResolver = new Bff.FragmentUnityResolver();

            foreach (
                Type solutionType in Assembly
                    .GetExecutingAssembly()
                    .GetTypes()
                    .Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(CommonSolution)))
            )
                arguments.Generate(solutionType);
        }

        private static void ConfigureAutoCleanup()
        {
            Util.FilesAutoCleanupActive = true;
            Util.FilesAutoCleanupDBPath = Path.Combine(Globals.TmpDirectory, "sharpmake");

            if (!Directory.Exists(Util.FilesAutoCleanupDBPath))
                Directory.CreateDirectory(Util.FilesAutoCleanupDBPath);
        }
    }
}
