"use client"

import { Course } from "@/Types/CourseTypes"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import {
  ArrowUpRight, BookOpen, Calendar, ClipboardList,
  LayoutDashboard, MoreVertical, UserCheck, Users,
} from "lucide-react"
import Link from "next/link"

interface Props {
  profile: any | null
  myCourses: Course[]
}

export function InstructorStateGrid({ profile, myCourses }: Props) {
  const totalStudents = myCourses.reduce((acc, curr) => acc + curr.students.length, 0)

  const statCards = [
    {
      title: "My Courses",
      value: myCourses.length,
      icon: BookOpen,
      gradient: "from-blue-600 to-cyan-500",
      href: "/instructor/courses",
    },
    {
      title: "Total Students",
      value: totalStudents,
      icon: Users,
      gradient: "from-indigo-600 to-purple-500",
      href: "/instructor/students",
    },
    {
      title: "Avg. Class Size",
      value: myCourses.length > 0 ? Math.round(totalStudents / myCourses.length) : 0,
      icon: UserCheck,
      gradient: "from-orange-600 to-rose-500",
      href: "#",
    },
  ]

  return (
    <div className="space-y-8 animate-in fade-in slide-in-from-bottom-2 duration-700">
      {/* Header */}
      <div className="flex flex-col gap-4 md:flex-row md:items-center md:justify-between">
        <div>
          <h1 className="text-3xl font-black tracking-tight text-foreground md:text-4xl">
            Hello,{" "}
            <span className="text-indigo-600">
              Prof. {profile?.full_name?.split(" ").pop() || "Instructor"}
            </span>
          </h1>
          <p className="font-medium text-muted-foreground">
            Managing{" "}
            <span className="font-bold text-foreground">{myCourses.length}</span>{" "}
            active course sections today.
          </p>
        </div>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-2 gap-4 lg:grid-cols-4">
        {statCards.map((stat) => (
          <Link
            key={stat.title}
            href={stat.href}
            className="group relative overflow-hidden rounded-2xl border border-border bg-card p-5 transition-all duration-300 hover:border-indigo-300 hover:shadow-2xl hover:shadow-indigo-100"
          >
            <div className="flex flex-col gap-4">
              <div
                className={`flex h-12 w-12 items-center justify-center rounded-2xl bg-gradient-to-br ${stat.gradient} shadow-lg transition-transform`}
              >
                <stat.icon className="h-6 w-6 text-white" />
              </div>
              <div>
                <p className="text-xs font-bold uppercase tracking-widest text-muted-foreground">
                  {stat.title}
                </p>
                <p className="text-3xl font-black tracking-tighter text-foreground">
                  {stat.value}
                </p>
              </div>
            </div>
            <ArrowUpRight className="absolute right-4 top-4 h-5 w-5 text-muted-foreground/20 transition-all group-hover:text-indigo-600 group-hover:opacity-100" />
          </Link>
        ))}
      </div>

      {/* Course List */}
      <div className="grid gap-6 lg:grid-cols-1">
        <Card className="overflow-hidden rounded-3xl border-border shadow-sm lg:col-span-2">
          <CardHeader className="border-b bg-slate-50/50">
            <div className="flex items-center justify-between">
              <CardTitle className="flex items-center gap-2 text-xl font-black">
                <ClipboardList className="h-5 w-5 text-indigo-600" />
                Teaching Schedule
              </CardTitle>
              <Link
                href="/instructor/courses"
                className="rounded-full bg-indigo-50 px-3 py-1 text-xs font-bold text-indigo-600 hover:bg-indigo-100"
              >
                View All
              </Link>
            </div>
          </CardHeader>
          <CardContent className="p-0">
            {myCourses.length > 0 ? (
              <div className="divide-y">
                {myCourses.map((course) => (
                  <div
                    key={course.course_id}
                    className="flex items-center justify-between p-5 transition-colors hover:bg-slate-50/80"
                  >
                    <div className="space-y-1">
                      <p className="font-bold text-slate-900">{course.course_name}</p>
                      <div className="flex items-center gap-4 text-xs font-medium text-slate-500">
                        <span className="flex items-center gap-1">
                          <Users size={14} /> {course.students.length} Enrolled
                        </span>
                        <span className="flex items-center gap-1">
                          <LayoutDashboard size={14} /> {course.features.length} Modules
                        </span>
                      </div>
                    </div>
                    <button className="rounded-lg p-2 text-slate-400 hover:bg-slate-100 hover:text-slate-900">
                      <MoreVertical size={20} />
                    </button>
                  </div>
                ))}
              </div>
            ) : (
              <div className="flex flex-col items-center justify-center py-12">
                <BookOpen size={48} className="mb-2 text-slate-200" />
                <p className="font-medium text-slate-500">No assigned courses found.</p>
              </div>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  )
}