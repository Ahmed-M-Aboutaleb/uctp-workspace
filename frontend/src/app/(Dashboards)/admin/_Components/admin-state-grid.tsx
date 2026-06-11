// app/admin/_components/StatsGrid.tsx
"use client"

import { Sparkles, DoorOpen, Users, BookOpen, GraduationCap, ArrowUpRight } from "lucide-react"
import Link from "next/link"

interface Props {
  stats: {
    features: number
    rooms: number
    instructors: number
    courses: number
    students: number
  }
}

export function StatsGrid({ stats }: Props) {
  const cards = [
    { title: "Total Features",  value: stats.features,    icon: Sparkles,      href: "/admin/features",    gradient: "from-primary to-primary/70" },
    { title: "Active Rooms",    value: stats.rooms,       icon: DoorOpen,      href: "/admin/rooms",       gradient: "from-accent to-accent/70" },
    { title: "Instructors",     value: stats.instructors, icon: Users,         href: "/admin/instructors", gradient: "from-amber-500 to-amber-400" },
    { title: "Courses",         value: stats.courses,     icon: BookOpen,      href: "/admin/courses",     gradient: "from-emerald-500 to-emerald-400" },
    { title: "Students",        value: stats.students,    icon: GraduationCap, href: "/admin/students",    gradient: "from-rose-500 to-rose-400" },
  ]

  return (
    <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-5">
      {cards.map((stat) => (
        <Link
          key={stat.title}
          href={stat.href}
          className="group relative overflow-hidden rounded-2xl border border-border bg-card p-5 transition-all duration-300 hover:border-primary/50 hover:shadow-lg hover:shadow-primary/5"
        >
          <div className="flex items-start justify-between">
            <div className="space-y-2">
              <p className="text-sm font-medium text-muted-foreground">{stat.title}</p>
              <p className="text-3xl font-bold text-foreground">{stat.value}</p>
            </div>
            <div className={`flex h-12 w-12 items-center justify-center rounded-xl bg-gradient-to-br ${stat.gradient} shadow-lg transition-transform duration-300 group-hover:scale-110`}>
              <stat.icon className="h-6 w-6 text-white" />
            </div>
          </div>
          <ArrowUpRight className="absolute bottom-4 right-4 h-4 w-4 text-muted-foreground/0 transition-all duration-300 group-hover:text-primary group-hover:opacity-100" />
        </Link>
      ))}
    </div>
  )
}