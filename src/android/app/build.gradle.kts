// Copyright 2023 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

import de.undercouch.gradle.tasks.download.Download
import java.io.File
import org.gradle.api.tasks.Copy

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("de.undercouch.download") version "5.6.0"
    id("kotlin-parcelize")
    kotlin("plugin.serialization") version "2.1.0"
    id("androidx.navigation.safeargs.kotlin")
}

/**
 * Use the number of seconds/10 since Jan 1 2016 as the versionCode.
 * This lets us upload a new build at most every 10 seconds for the
 * next 680 years.
 */
val autoVersion = (((System.currentTimeMillis() / 1000) - 1451606400) / 10).toInt()
val abiFilter = listOf("arm64-v8a", "x86_64")

val downloadedJniLibsPath = "${project.layout.buildDirectory.get().asFile}/downloadedJniLibs"

@Suppress("UnstableApiUsage")
android {
    namespace = "io.github.borked3ds.android"

    compileSdk = 35
    ndkVersion = "27.2.12479018"

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_23
        targetCompatibility = JavaVersion.VERSION_23
    }

    kotlinOptions {
        jvmTarget = "23"
    }

    androidResources {
        generateLocaleConfig = true
    }

    packaging {
        // This is necessary for libadrenotools custom driver loading
        jniLibs.useLegacyPackaging = true
    }

    buildFeatures {
        viewBinding = true
        buildConfig = true
    }

    lint {
        // This is important as it will run lint but not abort on error
        // Lint has some overly obnoxious "errors" that should really be warnings
        abortOnError = false
    }

    defaultConfig {
        // TODO If this is ever modified, change application_id in strings.xml
        applicationId = "io.github.borked3ds.android"
        minSdk = 28
        targetSdk = 35
        versionCode = autoVersion
        versionName = getGitVersion()

        ndk {
            //noinspection ChromeOsAbiSupport
            abiFilters += abiFilter
        }

        externalNativeBuild {
            cmake {
                arguments(
                    "-DENABLE_QT=0", // Don't use QT
                    "-DENABLE_SDL2=0", // Don't use SDL
                    "-DCMAKE_CXX_FLAGS=-O2",
                    "-DCMAKE_C_FLAGS=-O2",
                    "-DCMAKE_EXE_LINKER_FLAGS=-flto=thin",    // Enable Thin LTO
                    "-DCMAKE_SHARED_LINKER_FLAGS=-flto=thin", // Enable Thin LTO
                    "-DANDROID_ARM_NEON=true", // cryptopp requires Neon to work
                    "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON", // Support Android V 16KiB page sizes
                    "-DUSE_SYSTEM_BOOST=OFF",
                    "-DUSE_SYSTEM_CATCH2=OFF",
                    "-DUSE_SYSTEM_CRYPTOPP=OFF",
                    "-DUSE_SYSTEM_FMT=OFF",
                    "-DUSE_SYSTEM_XBYAK=OFF",
                    "-DUSE_SYSTEM_DYNARMIC=OFF",
                    "-DUSE_SYSTEM_INIH=OFF",
                    "-DUSE_SYSTEM_FFMPEG_HEADERS=OFF",
                    "-DUSE_SYSTEM_SOUNDTOUCH=OFF",
                    "-DUSE_SYSTEM_LIBUSB=OFF",
                    "-DUSE_SYSTEM_ZSTD=OFF",
                    "-DUSE_SYSTEM_ENET=OFF",
                    "-DUSE_SYSTEM_CUBEB=OFF",
                    "-DUSE_SYSTEM_JSON=OFF",
                    "-DUSE_SYSTEM_OPENSSL=OFF",
                    "-DUSE_SYSTEM_CPP_HTTPLIB=OFF",
                    "-DUSE_SYSTEM_CPP_JWT=OFF",
                    "-DUSE_SYSTEM_LODEPNG=OFF",
                    "-DUSE_SYSTEM_OPENAL=OFF",
                    "-DUSE_SYSTEM_GLSLANG=OFF",
                    "-DUSE_SYSTEM_VULKAN_HEADERS=OFF",
                    "-DUSE_SYSTEM_VMA=OFF",
                    "-DBORKED3DS_USE_EXTERNAL_VULKAN_SPIRV_TOOLS=ON"
                )
            }
        }

        buildConfigField("String", "GIT_HASH", "\"${getGitHash()}\"")
        buildConfigField("String", "BRANCH", "\"${getBranch()}\"")
    }

    val keystoreFile = System.getenv("DROID_KEYSTORE_FILE")
    if (keystoreFile != null) {
        signingConfigs {
            create("release") {
                storeFile = file(keystoreFile)
                storePassword = System.getenv("DROID_KEYSTORE_PASS")
                keyAlias = System.getenv("DROID_KEY_ALIAS")
                keyPassword = System.getenv("DROID_KEY_PASS")
            }
        }
    }

    // Define build types, which are orthogonal to product flavors.
    buildTypes {
        // Signed by release key, allowing for upload to Play Store.
        release {
            signingConfig = if (keystoreFile != null) {
                signingConfigs.getByName("release")
            } else {
                signingConfigs.getByName("debug")
            }
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android.txt"),
                "proguard-rules.pro"
            )
        }

        // builds a release build that doesn't need signing
        // Attaches 'debug' suffix to version and package name, allowing installation alongside the release build.
        register("relWithDebInfo") {
            initWith(getByName("release"))
            applicationIdSuffix = ".debug"
            versionNameSuffix = "-debug"
            signingConfig = signingConfigs.getByName("debug")
            isMinifyEnabled = true 
            isShrinkResources = true
            isDebuggable = false
            isJniDebuggable = false
            proguardFiles(
                getDefaultProguardFile("proguard-android.txt"),
                "proguard-rules.pro"
            )
            isDefault = true
        }

        // Signed by debug key disallowing distribution on Play Store.
        // Attaches 'debug' suffix to version and package name, allowing installation alongside the release build.
        debug {
            // TODO If this is ever modified, change application_id in debug/strings.xml
            applicationIdSuffix = ".debug"
            versionNameSuffix = "-debug"
            isDebuggable = true
            isJniDebuggable = true
        }
    }

    flavorDimensions.add("version")

    externalNativeBuild {
        cmake {
            version = "3.30.6"
            path = file("../../../CMakeLists.txt")
        }
    }

    sourceSets {
        named("main") {
            // Set up path for downloaded native libraries
            jniLibs.srcDir(downloadedJniLibsPath)
        }
    }
}

dependencies {
    implementation("androidx.activity:activity-ktx:1.10.0")
    implementation("androidx.appcompat:appcompat:1.7.0")
    implementation("androidx.core:core-ktx:1.15.0")
    implementation("androidx.core:core-splashscreen:1.0.1")
    implementation("androidx.documentfile:documentfile:1.0.1")
    implementation("androidx.fragment:fragment-ktx:1.8.5")
    implementation("androidx.lifecycle:lifecycle-viewmodel-ktx:2.8.7")
    implementation("androidx.navigation:navigation-fragment-ktx:2.8.5")
    implementation("androidx.navigation:navigation-ui-ktx:2.8.5")
    implementation("androidx.preference:preference-ktx:1.2.1")
    implementation("androidx.recyclerview:recyclerview:1.4.0")
    implementation("androidx.slidingpanelayout:slidingpanelayout:1.2.0")
    implementation("androidx.swiperefreshlayout:swiperefreshlayout:1.1.0")
    implementation("androidx.work:work-runtime:2.10.0")
    implementation("com.google.android.material:material:1.12.0")
    implementation("info.debatty:java-string-similarity:2.0.0")
    implementation("io.coil-kt:coil:2.7.0")
    implementation("org.ini4j:ini4j:0.5.4")
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.8.0")
}

// Download Vulkan Validation Layers from the KhronosGroup GitHub.
val downloadVulkanValidationLayers = tasks.register<Download>("downloadVulkanValidationLayers") {
    src("https://github.com/KhronosGroup/Vulkan-ValidationLayers/releases/download/vulkan-sdk-1.4.304.0/android-binaries-1.4.304.0.zip")
    dest(file("${project.layout.buildDirectory.get().asFile}/tmp/Vulkan-ValidationLayers.zip"))
    onlyIfModified(true)
}

// Extract Vulkan Validation Layers into the downloaded native libraries directory.
val unzipVulkanValidationLayers = tasks.register<Copy>("unzipVulkanValidationLayers") {
    val filePath = "${project.layout.buildDirectory.get().asFile}/tmp/Vulkan-ValidationLayers.zip"
    val file = File(filePath)
    if (!file.exists()) {
        dependsOn(downloadVulkanValidationLayers)
    }
    from(zipTree("${project.layout.buildDirectory.get().asFile}/tmp/Vulkan-ValidationLayers.zip")) {
        // Exclude the top level directory in the zip as it violates the expected jniLibs directory structure.
        eachFile {
            relativePath = RelativePath(true, *relativePath.segments.drop(1).toTypedArray())
        }
        includeEmptyDirs = false
    }
    into(downloadedJniLibsPath)
}

tasks.named("preBuild") {
    dependsOn(unzipVulkanValidationLayers)
}

fun getGitVersion(): String {
    var versionName = "0.0"

    try {
        versionName = ProcessBuilder("git", "describe", "--always", "--long")
            .directory(project.rootDir)
            .redirectOutput(ProcessBuilder.Redirect.PIPE)
            .redirectError(ProcessBuilder.Redirect.PIPE)
            .start().inputStream.bufferedReader().use { it.readText() }
            .trim()
            .replace(Regex("(-0)?-[^-]+$"), "")
    } catch (e: Exception) {
        logger.error("Cannot find git, defaulting to dummy version number")
    }

    if (System.getenv("GITHUB_ACTIONS") != null) {
        val gitTag = System.getenv("GIT_TAG_NAME")
        versionName = gitTag ?: versionName
    }

    return versionName
}

fun getGitHash(): String =
    runGitCommand(ProcessBuilder("git", "rev-parse", "--short", "HEAD")) ?: "dummy-hash"

fun getBranch(): String =
    runGitCommand(ProcessBuilder("git", "rev-parse", "--abbrev-ref", "HEAD")) ?: "dummy-branch"

fun runGitCommand(command: ProcessBuilder): String? {
    try {
        command.directory(project.rootDir)
        val process = command.start()
        val inputStream = process.inputStream
        val errorStream = process.errorStream
        process.waitFor()

        return if (process.exitValue() == 0) {
            inputStream.bufferedReader()
                .use { it.readText().trim() } // return the value of gitHash
        } else {
            val errorMessage = errorStream.bufferedReader().use { it.readText().trim() }
            logger.error("Error running git command: $errorMessage")
            return null
        }
    } catch (e: Exception) {
        logger.error("$e: Cannot find git")
        return null
    }
}

android.applicationVariants.configureEach {
    val variant = this
    val capitalizedName = variant.name.replaceFirstChar { it.uppercase() }

    val copyTask = tasks.register<Copy>("copyBundle${capitalizedName}") {
        // Define the copy specs at configuration time
        from(variant.outputs.first().outputFile.parentFile) {
            include("*.apk")
        }
        from(layout.buildDirectory.dir("outputs/bundle/${variant.name}")) {
            include("*.aab")
        }
        into(layout.buildDirectory.dir("bundle"))
    }
    
    tasks.named("bundle${capitalizedName}").configure { finalizedBy(copyTask) }
}
