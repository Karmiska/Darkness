// Copyright (c) 2020-2021 Ubisoft Entertainment
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
using System.Diagnostics;
using Sharpmake;

namespace HelloXCode
{
    [Fragment, Flags]
    public enum Optimization
    {
        Debug = 1 << 0,
        Release = 1 << 1
    }

    [Fragment, Flags]
    public enum BuildSystem
    {
        Default = 1 << 0,
        FastBuild = 1 << 1,
    }

    [DebuggerDisplay("\"{Platform}_{DevEnv}\" {Name}")]
    public class CommonTarget : Sharpmake.ITarget
    {
        public Platform Platform;
        public DevEnv DevEnv;
        public Optimization Optimization;
        public Blob Blob;
        public BuildSystem BuildSystem;

        public CommonTarget() { }

        public CommonTarget(
            Platform platform,
            DevEnv devEnv,
            Optimization optimization,
            Blob blob,
            BuildSystem buildSystem
        )
        {
            Platform = platform;
            DevEnv = devEnv;
            Optimization = optimization;
            Blob = blob;
            BuildSystem = buildSystem;
        }

        public override string Name
        {
            get
            {
                var nameParts = new List<string>();

                nameParts.Add(Optimization.ToString().ToLowerInvariant());

                return string.Join(" ", nameParts);
            }
        }

        public string SolutionPlatformName
        {
            get
            {
                var nameParts = new List<string>();

                nameParts.Add(BuildSystem.ToString());

                if (BuildSystem == BuildSystem.FastBuild && Blob == Blob.NoBlob)
                    nameParts.Add(Blob.ToString());

                return string.Join("_", nameParts);
            }
        }

        /// <summary>
        /// returns a string usable as a directory name, to use for instance for the intermediate path
        /// </summary>
        public string DirectoryName
        {
            get
            {
                var dirNameParts = new List<string>();

                dirNameParts.Add(Platform.ToString());
                dirNameParts.Add(Optimization.ToString());

                if (BuildSystem == BuildSystem.FastBuild)
                    dirNameParts.Add(BuildSystem.ToString());

                if (DevEnv != DevEnv.xcode)
                    dirNameParts.Add(DevEnv.ToString());

                return string.Join("_", dirNameParts).ToLowerInvariant();
            }
        }

        public override Platform GetPlatform()
        {
            return Platform;
        }

        public static CommonTarget[] GetDefaultTargets()
        {
            var result = new List<CommonTarget>();
            result.AddRange(GetMacTargets());
            return result.ToArray();
        }

        public static CommonTarget[] GetMacTargets()
        {
            var macosTarget = new CommonTarget(
                Platform.mac,
                DevEnv.xcode,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob,
                BuildSystem.Default
            );

            // make a blob version of the target
            var macosBlobTarget = (CommonTarget)macosTarget.Clone(
                Blob.Blob
            );

            // make a fastbuild version of the target
            var macosFastBuildTarget = (CommonTarget)macosTarget.Clone(
                Blob.FastBuildUnitys,
                BuildSystem.FastBuild
            );

            var iosTarget = new CommonTarget(
                Platform.ios,
                DevEnv.xcode,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob,
                BuildSystem.Default
            );

            // make a blob version of the target
            var iosBlobTarget = (CommonTarget)iosTarget.Clone(
                Blob.Blob
            );

            // make a fastbuild version of the target
            var iosFastBuildTarget = (CommonTarget)iosTarget.Clone(
                Blob.FastBuildUnitys,
                BuildSystem.FastBuild
            );

            var tvosTarget = new CommonTarget(
                Platform.tvos,
                DevEnv.xcode,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob,
                BuildSystem.Default
            );

            // make a fastbuild version of the target
            var tvosBlobTarget = (CommonTarget)tvosTarget.Clone(
                Blob.Blob
            );

            // make a fastbuild version of the target
            var tvosFastBuildTarget = (CommonTarget)tvosTarget.Clone(
                Blob.FastBuildUnitys,
                BuildSystem.FastBuild
            );

            var watchosTarget = new CommonTarget(
                Platform.watchos,
                DevEnv.xcode,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob,
                BuildSystem.Default
            );

            // make a blob version of the target
            var watchosBlobTarget = (CommonTarget)watchosTarget.Clone(
                Blob.Blob
            );

            // make a fastbuild version of the target
            var watchosFastBuildTarget = (CommonTarget)watchosTarget.Clone(
                Blob.FastBuildUnitys,
                BuildSystem.FastBuild
            );

            var catalystTarget = new CommonTarget(
                Platform.maccatalyst,
                DevEnv.xcode,
                Optimization.Debug | Optimization.Release,
                Blob.NoBlob,
                BuildSystem.Default
            );

            // make a blob version of the target
            var catalystBlobTarget = (CommonTarget)catalystTarget.Clone(
                Blob.Blob
            );

            // make a FastBuild version of the target
            var catalystFastBuildTarget = (CommonTarget)catalystTarget.Clone(
                Blob.FastBuildUnitys,
                BuildSystem.FastBuild
            );

            return new[] {
                macosTarget, macosBlobTarget, macosFastBuildTarget,
                iosTarget, iosBlobTarget, iosFastBuildTarget,
                tvosTarget, tvosBlobTarget, tvosFastBuildTarget,
                watchosTarget, watchosBlobTarget, watchosFastBuildTarget,
                catalystTarget, catalystBlobTarget, catalystFastBuildTarget,
            };
        }
    }
}
