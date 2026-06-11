"use client"

import { useLogout } from "@/hooks/useLogout"
import { InstructorRecord } from "@/Types/StudentTypes"
import {
  Calendar,
  ChevronLeft,
  ChevronRight,
  GraduationCap,
  LayoutDashboard,
  LogOut,
  Menu,

  User,

  X
} from "lucide-react"
import Link from "next/link"
import logo from "@/assets/download.png"
import { usePathname } from "next/navigation"
import { useState } from "react"
import Image from "next/image"

const navItems = [
  { href: "/instructor", label: "Dashboard", icon: LayoutDashboard },
  { href: "/instructor/timetable", label: "Timetable", icon: Calendar },
]


export function InstructorSidebar({ profile }: { profile: InstructorRecord }) {
  
   const pathname = usePathname()
  const [collapsed, setCollapsed] = useState(false)
  const [mobileOpen, setMobileOpen] = useState(false);
  const {isLoggingOut,logout}=useLogout();

  return (
    <>
      {/* Mobile menu button */}
      <button
        onClick={() => setMobileOpen(true)}
        className="fixed left-4 top-4 z-50 flex h-10 w-10 items-center justify-center rounded-lg bg-gradient-to-r from-indigo-600 to-violet-500 text-primary-foreground shadow-lg md:hidden"
        aria-label="Open menu"
      >
        <Menu className="h-5 w-5" />
      </button>

      {/* Mobile overlay */}
      {mobileOpen && (
        <div
          className="fixed inset-0 z-40 bg-black/50 backdrop-blur-sm md:hidden"
          onClick={() => setMobileOpen(false)}
        />
      )}

      {/* Sidebar */}
      <aside
        className={`fixed left-0 top-0 z-50 flex h-full flex-col border-r border-border bg-card transition-all duration-300 ${
          collapsed ? "w-[72px]" : "w-64"
        } ${mobileOpen ? "translate-x-0" : "-translate-x-full"} md:translate-x-0`}
      >
        {/* Header */}
        <div className="flex h-16 items-center justify-between border-b border-border px-4">
          {!collapsed && (
            <Link href="/instructor" className="flex items-center gap-3">
              <Image src={logo} alt="EduAdmin Logo" className="h-12 w-12" />
              <div className="flex flex-col">
                <span className="text-lg font-bold text-foreground">Instructor Portal</span>
              </div>
            </Link>
          )}
          {collapsed && (
            <div className="mx-auto flex h-10 w-10 items-center justify-center rounded-xl  shadow-md">
              <Image src={logo} alt="EduAdmin Logo" className="h-12 w-12" />
            </div>
          )}
          <button
            onClick={() => setMobileOpen(false)}
            className="flex h-8 w-8 items-center justify-center rounded-lg text-muted-foreground hover:bg-muted md:hidden"
            aria-label="Close menu"
          >
            <X className="h-5 w-5" />
          </button>
        </div>

        {/* Navigation */}
        <nav className="flex-1 overflow-y-auto p-3">
          <p className={`mb-2 px-3 text-xs font-semibold uppercase tracking-wider text-muted-foreground ${collapsed ? "sr-only" : ""}`}>
            Main Menu
          </p>
          <ul className="space-y-1">
            {navItems.map((item) => {
              const isActive = pathname === item.href
              const Icon = item.icon
              return (
                <li key={item.href}>
                  <Link
                    href={item.href}
                    onClick={() => setMobileOpen(false)}
                    className={`group flex items-center gap-3 rounded-xl px-3 py-2.5 text-sm font-medium transition-all duration-200 ${
                      isActive
                        ? "bg-gradient-to-r from-indigo-600 to-violet-500 text-primary-foreground shadow-md"
                        : "text-muted-foreground hover:bg-muted hover:text-foreground"
                    } ${collapsed ? "justify-center px-2" : ""}`}
                    title={collapsed ? item.label : undefined}
                  >
                    <Icon className={`h-5 w-5 flex-shrink-0 transition-transform duration-200 ${!isActive ? "group-hover:scale-110" : ""}`} />
                    {!collapsed && <span>{item.label}</span>}
                  </Link>
                </li>
              )
            })}
          </ul>
        </nav>

        {/* Footer */}
        <div className="border-t border-border p-3">
          <div className={`flex items-center gap-3 rounded-xl bg-muted/50 p-3 ${collapsed ? "justify-center" : ""}`}>
            <Link href={'/instructor/profile'} className="flex cursor-pointer h-9 w-9 flex-shrink-0 items-center justify-center rounded-full bg-gradient-to-r from-indigo-600 to-violet-500 font-semibold text-white shadow-sm">
              <User className="h-4 w-4" />
            </Link>
            {!collapsed && (
              <div className="flex-1 overflow-hidden">
                <p className="truncate text-sm font-medium text-foreground">{profile.name.toUpperCase()}</p>
                <p className="truncate text-xs text-muted-foreground">ID-{profile.user_id}</p>
              </div>
            )}
            {!collapsed && (
              <button className="rounded-lg cursor-pointer p-2 text-muted-foreground transition-colors hover:bg-background hover:text-foreground"
                onClick={logout}
                disabled={isLoggingOut}
              >
                <LogOut className="h-4 w-4" />
              </button>
            )}
          </div>

          {/* Collapse button - desktop only */}
          <button
            onClick={() => setCollapsed(!collapsed)}
            className="mt-3 hidden w-full items-center justify-center gap-2 rounded-xl border border-border bg-background px-3 py-2 text-sm text-muted-foreground transition-colors hover:bg-muted hover:text-foreground md:flex"
          >
            {collapsed ? (
              <ChevronRight className="h-4 w-4" />
            ) : (
              <>
                <ChevronLeft className="h-4 w-4" />
                <span>Collapse</span>
              </>
            )}
          </button>
        </div>
      </aside>

      {/* Spacer for main content */}
      <div className={`hidden flex-shrink-0 transition-all duration-300 md:block ${collapsed ? "w-[72px]" : "w-64"}`} />
    </>
  )

}
