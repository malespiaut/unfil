const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "unfil",
        .target = target,
        .optimize = optimize,
    });

    exe.addCSourceFiles(&.{
        "src/unfil.c",
    }, &.{
        "-std=c17",
        "-Wall",
        "-Wextra",
        "-Wpedantic",
        "-Wshadow",
    });

    exe.linkSystemLibrary("c");
    b.installArtifact(exe);
}
