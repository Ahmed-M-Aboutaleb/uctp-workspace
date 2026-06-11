"use client"

import { Course } from "@/Types/CourseTypes"
import { StudentRecord } from "@/Types/StudentTypes"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import {
  ArrowUpRight, BookOpen, Calendar,
  CheckCircle2, Clock, GraduationCap,
} from "lucide-react"
import Link from "next/link"

interface Props {
  student: StudentRecord | null
  courses: Course[]
}

export function StudentStateGrid({ student, courses }: Props) {
  const instructorCount = new Set(
    courses.flatMap((c) => c.instructors.map((i) => i.instructor_id))
  ).size

  const statCards = [
    {
      title: "Enrolled Courses",
      value: courses.length,
      icon: BookOpen,
      gradient: "from-blue-500 to-indigo-500",
      href: "/student/enrollment",
    },
    {
      title: "Current Semester",
      value: student?.semester || "N/A",
      icon: GraduationCap,
      gradient: "from-emerald-500 to-teal-500",
      href: "#",
    },
    {
      title: "Your Timetable",
      value: "",
      icon: Calendar,
      gradient: "from-purple-500 to-pink-500",
      href: "/student/timetable",
    },
    {
      title: "Instructor Count",
      value: instructorCount,
      icon: Clock,
      gradient: "from-orange-500 to-amber-500",
      href: "#",
    },
  ]

  return (
    <div className="space-y-8 animate-in fade-in duration-700">
      {/* Header */}
      <div className="flex flex-col gap-2 md:flex-row md:items-center md:justify-between">
        <div>
          <h1 className="text-3xl font-black tracking-tight text-foreground md:text-4xl">
            Welcome back,{" "}
            <span className="text-primary">
              {student?.full_name?.split(" ")[0] || "Student"}!
            </span>
          </h1>
          <p className="text-muted-foreground">
            You are currently tracking{" "}
            <span className="font-bold text-foreground">{courses.length}</span>{" "}
            active courses for the {student?.semester} semester.
          </p>
        </div>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-2 gap-4 lg:grid-cols-4">
        {statCards.map((stat) => (
          <Link
            key={stat.title}
            href={stat.href}
            className="group relative overflow-hidden rounded-2xl border border-border bg-card p-5 transition-all duration-300 hover:border-primary/50 hover:shadow-xl"
          >
            <div className="flex flex-col gap-4">
              <div
                className={`flex h-10 w-10 items-center justify-center rounded-xl bg-gradient-to-br ${stat.gradient} shadow-lg transition-transform group-hover:scale-110`}
              >
                <stat.icon className="h-5 w-5 text-white" />
              </div>
              <div>
                <p className="text-xs font-bold uppercase tracking-wider text-muted-foreground">
                  {stat.title}
                </p>
                <p className="text-2xl font-black text-foreground">{stat.value}</p>
              </div>
            </div>
            <ArrowUpRight className="absolute right-4 top-4 h-4 w-4 text-muted-foreground/30 transition-all group-hover:text-primary group-hover:opacity-100" />
          </Link>
        ))}
      </div>

      {/* Enrollments */}
      <Card className="rounded-2xl border-border shadow-sm">
        <CardHeader className="flex flex-row items-center justify-between">
          <CardTitle className="flex items-center gap-2 text-lg font-bold">
            <CheckCircle2 className="h-5 w-5 text-emerald-500" />
            Active Enrollments
          </CardTitle>
          <Link
            href="/student/enrollment"
            className="text-xs font-bold text-primary hover:underline"
          >
            View Catalog
          </Link>
        </CardHeader>
        <CardContent className="space-y-4">
          {courses.length > 0 ? (
            courses.map((course) => (
              <div
                key={course.course_id}
                className="flex items-center justify-between rounded-xl border border-border p-4 transition-colors hover:bg-muted/50"
              >
                <div>
                  <p className="font-bold text-foreground">{course.course_name}</p>
                  <p className="text-xs text-muted-foreground">
                    {course.instructors.map((i) => i.name).join(", ")}
                  </p>
                </div>
                <div className="flex gap-1">
                  {course.features.slice(0, 2).map((f) => (
                    <span
                      key={f.feature_id}
                      className="rounded bg-slate-100 px-2 py-0.5 text-[10px] font-bold uppercase"
                    >
                      {f.feature_name}
                    </span>
                  ))}
                </div>
              </div>
            ))
          ) : (
            <p className="py-8 text-center text-sm text-muted-foreground">
              No courses enrolled yet.
            </p>
          )}
        </CardContent>
      </Card>
    </div>
  )
}