"use client"

import { User } from "lucide-react"
import Link from "next/link"
import { usePathname } from "next/navigation"

const pageTitles: Record<string, string> = {
  "/admin": "Dashboard",
  "/admin/features": "Features",
  "/admin/rooms": "Rooms",
  "/admin/courses": "Courses",
  "/admin/timetable": "Timetable",
  "/admin/admins": "Admins",
  "/admin/instructors": "Instructors",
  "/admin/students": "Students",
}

export function AdminHeader() {
  const pathname = usePathname()
  const title = pageTitles[pathname] || "Dashboard"

  return (
    <header className="sticky top-0 z-30 flex h-16 items-center  justify-between border-b border-border bg-card/95 px-4 backdrop-blur-sm md:px-6">
      <div className="flex items-center gap-4 pl-14 md:pl-0 ">
        <h1 className="text-xl font-bold text-foreground">{title}</h1>
      </div>

      <div className="flex items-center gap-3">

        

        {/* Profile */}
        <Link href="/admin/profile" className="hidden h-10 w-10 items-center justify-center rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 font-semibold text-white shadow-sm md:flex">
          <User className="h-4 w-4" />  
        </Link>
      </div>
    </header>
  )
}
