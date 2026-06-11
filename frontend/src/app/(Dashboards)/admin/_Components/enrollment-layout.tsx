"use client"

import { useState } from "react"
import { toast } from "react-toastify"
import {
    ArrowLeft,
    Users,
    GraduationCap,
    ToggleLeft,
    ToggleRight,
    Loader2,
    BookMarked,
    Search,
    Hash,
    CalendarDays,
    ShieldCheck,
    ShieldOff,
    RefreshCw,
} from "lucide-react"
import { useRouter } from "next/navigation"

// Server Actions

// Types
import { Course } from "@/Types/CourseTypes"
import { StudentResponse } from "@/Types/StudentTypes"
import { ToggleEnrollment } from "@/ServerActions/Enrollment/ToggleEnrollment"
import { GetCourseEnrollments } from "@/ServerActions/Enrollment/GetCourseEnrollments"

interface EnrollmentLayoutProps {
    course: Course
    initialEnrollments: StudentResponse
}

const SEMESTER_COLORS: Record<string, string> = {
    Fall:   "bg-orange-50 text-orange-600",
    Spring: "bg-emerald-50 text-emerald-600",
    Summer: "bg-sky-50 text-sky-600",
}

export default function EnrollmentLayout({ course, initialEnrollments }: EnrollmentLayoutProps) {
    const router = useRouter()

    const [enrollments, setEnrollments] = useState<StudentResponse>(initialEnrollments)
    const [isOpen, setIsOpen] = useState<boolean>(() => {
    if (typeof window === "undefined") return false
    const saved = localStorage.getItem(`enrollment_open_${course.course_id}`)
    return saved !== null ? JSON.parse(saved) : false
})
    const [isToggling, setIsToggling] = useState(false)
    const [isRefreshing, setIsRefreshing] = useState(false)
    const [searchQuery, setSearchQuery] = useState("")

    const filtered = enrollments.filter(s =>
        s.full_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
        s.student_id.toString().includes(searchQuery) ||
        s.semester.toLowerCase().includes(searchQuery.toLowerCase())
    )

    const semesterCounts = enrollments.reduce<Record<string, number>>((acc, s) => {
        acc[s.semester] = (acc[s.semester] || 0) + 1
        return acc
    }, {})

        const handleToggle = async () => {
            setIsToggling(true)
            try {
                const result = await ToggleEnrollment(course.course_id, !isOpen)
                const newState = !isOpen
                setIsOpen(newState)
                localStorage.setItem(`enrollment_open_${course.course_id}`, JSON.stringify(newState))
                toast.success(result.message)
            } catch {
                toast.error("Failed to toggle enrollment status")
            } finally {
                setIsToggling(false)
            }
        }
    const handleRefresh = async () => {
        setIsRefreshing(true)
        try {
            const fresh = await GetCourseEnrollments(course.course_id)
            setEnrollments(fresh)
            toast.success("Enrollment list refreshed")
        } catch {
            toast.error("Failed to refresh enrollments")
        } finally {
            setIsRefreshing(false)
        }
    }

    return (
        <div className="max-w-5xl mx-auto p-4 md:p-8 space-y-6 md:space-y-8 animate-in fade-in duration-500">

            {/* Back button */}
            <button
                onClick={() => {router.back(); router.refresh()}}
                className="flex items-center gap-2 text-sm font-bold text-slate-500 hover:text-indigo-600 transition-colors group"
            >
                <ArrowLeft className="h-4 w-4 group-hover:-translate-x-1 transition-transform" />
                Back to Courses
            </button>

            {/* Header */}
            <div className="flex flex-col gap-6 md:flex-row md:items-start md:justify-between border-b border-slate-100 pb-8">
                <div className="flex items-center gap-4">
                    <div className="flex h-12 w-12 md:h-14 md:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-indigo-500 shadow-xl shadow-indigo-100">
                        <BookMarked className="h-6 w-6 md:h-7 md:w-7 text-white" />
                    </div>
                    <div>
                        <h1 className="text-xl md:text-2xl font-extrabold tracking-tight text-slate-900 leading-tight">
                            {course.course_name}
                        </h1>
                        <p className="text-xs md:text-sm text-slate-500 font-medium mt-0.5">
                            Course #{course.course_id.toString().padStart(3, "0")} · Enrollment Management
                        </p>
                        {/* Feature tags */}
                        {course.features && course.features.length > 0 && (
                            <div className="flex flex-wrap gap-1 mt-2">
                                {course.features.map(f => (
                                    <span key={f.feature_id} className="text-[9px] bg-slate-100 text-slate-500 px-1.5 py-0.5 rounded uppercase font-bold">
                                        {f.feature_name}
                                    </span>
                                ))}
                            </div>
                        )}
                    </div>
                </div>

                {/* Toggle enrollment open/close */}
                <button
                    onClick={handleToggle}
                    disabled={isToggling}
                    className={`flex w-full md:w-auto items-center justify-center gap-2.5 rounded-xl px-6 py-3 text-sm font-bold shadow-lg transition-all active:scale-95 disabled:opacity-60 cursor-pointer
                        ${isOpen
                            ? "bg-gradient-to-r from-emerald-500 to-emerald-400 text-white hover:shadow-emerald-200"
                            : "bg-gradient-to-r from-rose-500 to-rose-400 text-white hover:shadow-rose-200"
                        }`}
                >
                    {isToggling
                        ? <Loader2 className="h-4 w-4 animate-spin" />
                        : isOpen
                            ? <><ShieldCheck className="h-4 w-4" /> Enrollment Open</>
                            : <><ShieldOff className="h-4 w-4" /> Enrollment Closed</>
                    }
                </button>
            </div>

            {/* Stats row */}
            <div className="grid grid-cols-2 md:grid-cols-4 gap-3 md:gap-4">
                {/* Total */}
                <div className="rounded-2xl border border-slate-200 bg-white p-4 flex flex-col gap-1 shadow-sm">
                    <div className="flex items-center gap-2 text-xs font-bold text-slate-400 uppercase tracking-wider">
                        <Users className="h-3.5 w-3.5" /> Total
                    </div>
                    <p className="text-2xl font-extrabold text-slate-800">{enrollments.length}</p>
                    <p className="text-xs text-slate-400">enrolled students</p>
                </div>

                {/* Per semester */}
                {(["Fall", "Spring", "Summer"] as const).map(sem => (
                    <div key={sem} className="rounded-2xl border border-slate-200 bg-white p-4 flex flex-col gap-1 shadow-sm">
                        <div className={`w-fit text-[10px] font-bold px-2 py-0.5 rounded-full uppercase tracking-wider ${SEMESTER_COLORS[sem]}`}>
                            {sem}
                        </div>
                        <p className="text-2xl font-extrabold text-slate-800">{semesterCounts[sem] || 0}</p>
                        <p className="text-xs text-slate-400">students</p>
                    </div>
                ))}
            </div>

            {/* Student list card */}
            <div className="rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">

                {/* Toolbar */}
                <div className="p-4 md:p-6 border-b border-slate-100 bg-slate-50/30 flex flex-col sm:flex-row sm:items-center gap-3">
                    <div className="relative flex-1 sm:max-w-sm">
                        <Search className="absolute left-3.5 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
                        <input
                            type="search"
                            placeholder="Search by name, ID or semester..."
                            className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-indigo-500/10 focus:border-indigo-500 outline-none transition-all"
                            value={searchQuery}
                            onChange={e => setSearchQuery(e.target.value)}
                        />
                    </div>
                    <button
                        onClick={handleRefresh}
                        disabled={isRefreshing}
                        className="flex items-center justify-center gap-2 rounded-xl border border-slate-200 bg-white px-4 py-2.5 text-sm font-bold text-slate-600 hover:bg-slate-50 transition-all disabled:opacity-50 shrink-0"
                    >
                        <RefreshCw className={`h-4 w-4 ${isRefreshing ? "animate-spin" : ""}`} />
                        Refresh
                    </button>
                </div>

                {/* ── DESKTOP table ── */}
                <div className="hidden md:block overflow-x-auto">
                    <table className="w-full text-left">
                        <thead>
                            <tr className="bg-slate-50/50">
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">#</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Student</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Student ID</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Semester</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">User ID</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-slate-100">
                            {filtered.map((student, index) => (
                                <tr key={student.student_id} className="hover:bg-indigo-50/20 transition-colors">
                                    <td className="px-8 py-5 text-xs font-mono text-slate-300 font-bold">
                                        {String(index + 1).padStart(2, "0")}
                                    </td>
                                    <td className="px-8 py-5">
                                        <div className="flex items-center gap-3">
                                            <div className="flex h-9 w-9 shrink-0 items-center justify-center rounded-xl bg-gradient-to-br from-indigo-500 to-indigo-600 text-white text-sm font-bold shadow-sm">
                                                {student.full_name.charAt(0).toUpperCase()}
                                            </div>
                                            <span className="text-sm font-bold text-slate-800">{student.full_name}</span>
                                        </div>
                                    </td>
                                    <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">
                                        #{student.student_id.toString().padStart(3, "0")}
                                    </td>
                                    <td className="px-8 py-5">
                                        <span className={`inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[10px] font-bold uppercase tracking-wide ${SEMESTER_COLORS[student.semester]}`}>
                                            <CalendarDays className="h-3 w-3" />
                                            {student.semester}
                                        </span>
                                    </td>
                                    <td className="px-8 py-5 text-sm text-slate-400 font-mono">
                                        {student.user_id}
                                    </td>
                                </tr>
                            ))}
                        </tbody>
                    </table>

                    {filtered.length === 0 && (
                        <div className="flex flex-col items-center justify-center py-20 gap-3">
                            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-slate-100">
                                <GraduationCap className="h-7 w-7 text-slate-300" />
                            </div>
                            <p className="text-sm font-bold text-slate-400">No students found</p>
                            <p className="text-xs text-slate-400">Try adjusting your search query.</p>
                        </div>
                    )}
                </div>

                {/* ── MOBILE cards ── */}
                <div className="md:hidden divide-y divide-slate-100">
                    {filtered.length === 0 ? (
                        <div className="flex flex-col items-center justify-center py-20 gap-3">
                            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-slate-100">
                                <GraduationCap className="h-7 w-7 text-slate-300" />
                            </div>
                            <p className="text-sm font-bold text-slate-400">No students found</p>
                        </div>
                    ) : filtered.map((student, index) => (
                        <div key={student.student_id} className="p-4 hover:bg-indigo-50/20 transition-colors">
                            <div className="flex items-center gap-3">
                                <div className="flex h-10 w-10 shrink-0 items-center justify-center rounded-xl bg-gradient-to-br from-indigo-500 to-indigo-600 text-white text-sm font-bold shadow-sm">
                                    {student.full_name.charAt(0).toUpperCase()}
                                </div>
                                <div className="flex-1 min-w-0">
                                    <p className="text-sm font-bold text-slate-800 truncate">{student.full_name}</p>
                                    <div className="flex items-center gap-2 mt-0.5 flex-wrap">
                                        <span className="text-xs font-mono text-indigo-500">
                                            #{student.student_id.toString().padStart(3, "0")}
                                        </span>
                                        <span className={`inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[10px] font-bold uppercase tracking-wide ${SEMESTER_COLORS[student.semester]}`}>
                                            {student.semester}
                                        </span>
                                    </div>
                                </div>
                                <span className="text-[11px] font-mono text-slate-300 font-bold shrink-0">
                                    #{String(index + 1).padStart(2, "0")}
                                </span>
                            </div>
                        </div>
                    ))}
                </div>

                {/* Footer */}
                <div className="p-4 md:p-6 bg-slate-50/30 border-t border-slate-100">
                    <p className="text-sm text-slate-500">
                        Showing <span className="text-slate-900 font-bold">{filtered.length}</span> of <span className="text-slate-900 font-bold">{enrollments.length}</span> students
                    </p>
                </div>
            </div>
        </div>
    )
}