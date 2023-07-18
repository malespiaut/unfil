const std = @import("std");
const Build = std.build;

pub fn build(b: *Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "unfil",
        .optimize = optimize,
        .target = target,
    });
    exe.addCSourceFiles(&.{
        "src/unfil.c",
    }, &.{
        "-std=c17",
        "-Wpedantic",
        "-Wall",
        "-Wextra",
        "-Wshadow",
    });

    exe.addIncludePath("src");

    exe.linkSystemLibrary("c");
    b.installArtifact(exe);

    const run = b.addRunArtifact(exe);
    run.step.dependOn(b.getInstallStep());
}
