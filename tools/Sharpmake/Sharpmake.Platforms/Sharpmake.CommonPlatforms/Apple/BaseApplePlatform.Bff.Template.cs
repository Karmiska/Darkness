// Copyright (c) Ubisoft. All Rights Reserved.
// Licensed under the Apache 2.0 License. See LICENSE.md in the project root for license information.

namespace Sharpmake
{
    public abstract partial class BaseApplePlatform
    {
        private const string _linkerOptionsTemplate = @"
    .LinkerOptions          = '-o ""%2"" ""%1""[outputTypeArgument]'
                            // Library Search Path
                            // ---------------------------
                            + ' [cmdLineOptions.SysLibRoot]'
                            + ' [cmdLineOptions.AdditionalLibraryDirectories]'
                            // Libraries
                            // ---------------------------
                            + ' [cmdLineOptions.AdditionalDependencies]'
                            // SystemFrameworks, DeveloperFrameworks, UserFrameworks and FrameworkPaths
                            // -----------------------------------------------------------------------------
                            + ' [cmdLineOptions.SystemFrameworks]'
                            + ' [cmdLineOptions.DeveloperFrameworks]'
                            + ' [cmdLineOptions.UserFrameworks]'
                            + ' [cmdLineOptions.EmbeddedFrameworks]'
                            + ' [cmdLineOptions.LinkerSystemFrameworkPaths]'
                            + ' [cmdLineOptions.LinkerFrameworkPaths]'
                            // Options
                            //--------
                            + ' [cmdLineOptions.GenerateMapFile]'
                            // Additional linker options
                            //--------------------------
                            + ' [options.AdditionalLinkerOptions]'
";

        private const string _compilerExtraOptionsGeneral = @"
    .CompilerExtraOptions   = ''
            // General options
            // -------------------------
            + ' [cmdLineOptions.AdditionalIncludeDirectories]'
            + ' [cmdLineOptions.AdditionalUsingDirectories]'
            + ' [cmdLineOptions.PreprocessorDefinitions]'
            + ' [cmdLineOptions.StdLib]'
            + ' [cmdLineOptions.SDKRoot]'
            + ' [options.ClangCppLanguageStandard]'
";

        private const string _compilerExtraOptionsAdditional = @"
            // Additional compiler options
            //--------------------------
            + ' [options.AdditionalCompilerOptions]'
            // SystemFrameworks, DeveloperFrameworks, UserFrameworks and FrameworkPaths
            // ----------------------------------------------------------------------------
            + ' [cmdLineOptions.SystemFrameworks]'
            + ' [cmdLineOptions.DeveloperFrameworks]'
            + ' [cmdLineOptions.UserFrameworks]'
            + ' [cmdLineOptions.EmbeddedFrameworks]'
            + ' [cmdLineOptions.CompilerSystemFrameworkPaths]'
            + ' [cmdLineOptions.CompilerFrameworkPaths]'
";

        private const string _compilerOptimizationOptions =
                @"
    // Optimizations options
    // ---------------------
    .CompilerOptimizations = ''
            + ' [cmdLineOptions.OptimizationLevel]'
            + ' [cmdLineOptions.GenerateDebuggingSymbols]'
            + ' [options.AdditionalCompilerOptimizeOptions]'
";
    }
}
