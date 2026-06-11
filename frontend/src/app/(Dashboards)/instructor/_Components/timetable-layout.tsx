"use client"

import { GetTimetable } from "@/ServerActions/Timetable/GetTimetable"
import { Course } from "@/Types/CourseTypes"
import { InstructorRecord } from "@/Types/StudentTypes"
import { TimetableResponse } from "@/Types/TimetableTypes"
import {
    BookOpen,
    Calendar,
    ChevronLeft,
    ChevronRight,
    Clock,
    Layers,
    LayoutGrid,
    Loader2,
    MapPin,
    RefreshCw,
    Search,
    User,
} from "lucide-react"
import { useMemo, useState } from "react"
import { toast } from "react-toastify"

interface InstructorTimetableLayoutProps {
    initialTimetable: TimetableResponse
    instructor: InstructorRecord
}

const DAYS = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
const HOURS = ["8:00 AM", "9:00 AM", "10:00 AM", "11:00 AM", "12:00 PM", "1:00 PM", "2:00 PM", "3:00 PM", "4:00 PM"]

const COURSE_COLORS = [
    { bg: "bg-violet-50",  border: "border-violet-200",  text: "text-violet-700",  accent: "text-violet-500" },
    { bg: "bg-sky-50",     border: "border-sky-200",     text: "text-sky-700",     accent: "text-sky-500"    },
    { bg: "bg-emerald-50", border: "border-emerald-200", text: "text-emerald-700", accent: "text-emerald-500"},
    { bg: "bg-amber-50",   border: "border-amber-200",   text: "text-amber-700",   accent: "text-amber-500"  },
    { bg: "bg-rose-50",    border: "border-rose-200",    text: "text-rose-700",    accent: "text-rose-500"   },
    { bg: "bg-fuchsia-50", border: "border-fuchsia-200", text: "text-fuchsia-700", accent: "text-fuchsia-500"},
    { bg: "bg-teal-50",    border: "border-teal-200",    text: "text-teal-700",    accent: "text-teal-500"   },
    { bg: "bg-orange-50",  border: "border-orange-200",  text: "text-orange-700",  accent: "text-orange-500" },
]

export default function InstructorTimetableLayout({
    initialTimetable,
    instructor,
}: InstructorTimetableLayoutProps) {
    const [timetable, setTimetable]             = useState<TimetableResponse>(initialTimetable)
    const [isRefreshing, setIsRefreshing]       = useState(false)
    const [searchQuery, setSearchQuery]         = useState("")
    const [activeDayMobile, setActiveDayMobile] = useState(0)

    const studentCourseIds = useMemo(() => {
        const ids = new Set<number>()
        timetable.forEach(item => {
            const course: Course = item.course
            if (course.instructors.some(s => s.instructor_id === instructor.instructor_id))
                ids.add(course.course_id)
        })
        return ids
    }, [timetable, instructor.instructor_id])

    const courseColorMap = useMemo(() => {
        const map = new Map<number, (typeof COURSE_COLORS)[0]>()
        let i = 0
        timetable.forEach(item => {
            const { course_id } = item.course
            if (studentCourseIds.has(course_id) && !map.has(course_id)) {
                map.set(course_id, COURSE_COLORS[i % COURSE_COLORS.length])
                i++
            }
        })
        return map
    }, [timetable, studentCourseIds])

    const studentTimetable = useMemo(
        () => timetable.filter(item => studentCourseIds.has(item.course.course_id)),
        [timetable, studentCourseIds],
    )

    const handleRefresh = async () => {
        setIsRefreshing(true)
        try {
            const fresh = await GetTimetable()
            setTimetable(fresh)
            toast.success("Schedule updated")
        } catch {
            toast.error("Failed to refresh schedule")
        } finally {
            setIsRefreshing(false)
        }
    }

    const getEntryAt = (dayIdx: number, hourIdx: number) => {
        const timeslotId = dayIdx * 9 + hourIdx
        const entry = studentTimetable.find(item => item.timeslot_id === timeslotId)
        if (!entry) return { entry: null, isMatch: true, id: timeslotId, color: null }
        const color = courseColorMap.get(entry.course.course_id) ?? COURSE_COLORS[0]
        const isMatch =
            searchQuery === "" ||
            entry.course.course_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
            entry.room.room_id.toString().includes(searchQuery)
        return { entry, isMatch, id: timeslotId, color }
    }

    const enrolledCount = courseColorMap.size
    const totalSlots    = studentTimetable.length
    const prevDay = () => setActiveDayMobile(d => (d - 1 + DAYS.length) % DAYS.length)
    const nextDay = () => setActiveDayMobile(d => (d + 1) % DAYS.length)

    return (
        /*
         * KEY: contain=strict on the wrapper + explicit 100vw max cap.
         * overflow-x:hidden alone can't stop the *document* from scrolling
         * if a child has a larger natural width. We also set contain:layout
         * so absolutely-positioned descendants don't escape the box model.
         */
        <div style={{
            width: "100%",
            maxWidth: "100vw",
            boxSizing: "border-box",
            overflowX: "clip",   /* "clip" (not hidden) — doesn't create a scroll container */
            contain: "layout",
        }}>
            {/* inner padding wrapper — border-box so padding never adds width */}
            <div style={{
                width: "100%",
                maxWidth: 1600,
                margin: "0 auto",
                boxSizing: "border-box",
                padding: "12px",
                display: "flex",
                flexDirection: "column",
                gap: 16,
            }}>

                {/* ── BANNER ── */}
                <div style={{
                    width: "100%",
                    boxSizing: "border-box",
                    borderRadius: 16,
                    background: "linear-gradient(to right, #4f46e5, #6d28d9)",
                    padding: "14px 16px",
                    overflow: "hidden",   /* clips blobs precisely */
                    position: "relative",
                }}>
                    {/* blobs only on sm+ — on mobile they were leaking layout width */}
                    <div className="hidden sm:block" style={{
                        position: "absolute", top: -40, right: -40,
                        width: 192, height: 192, borderRadius: "50%",
                        background: "rgba(99,102,241,0.15)",
                        filter: "blur(40px)", pointerEvents: "none",
                    }} />

                    {/* single-row layout — no flex-col on any breakpoint */}
                    <div style={{ display: "flex", alignItems: "center", gap: 12, width: "100%", boxSizing: "border-box" }}>
                        {/* avatar */}
                        <div style={{
                            flexShrink: 0,
                            width: 40, height: 40,
                            borderRadius: 12,
                            background: "rgba(255,255,255,0.2)",
                            display: "flex", alignItems: "center", justifyContent: "center",
                            color: "white", fontWeight: 900, fontSize: 16,
                            userSelect: "none",
                        }}>
                            {(instructor.name ?? "I").charAt(0).toUpperCase()}
                        </div>

                        {/* text — min-w-0 is critical for truncation inside flex */}
                        <div style={{ flex: 1, minWidth: 0 }}>
                            <p style={{ fontSize: 9, fontWeight: 700, color: "#a5b4fc", marginBottom: 2, textTransform: "uppercase" }}>
                                Instructor Schedule
                            </p>
                            <h1 style={{
                                fontSize: 14, fontWeight: 800, color: "white",
                                whiteSpace: "nowrap", overflow: "hidden", textOverflow: "ellipsis",
                                lineHeight: 1.2,
                            }}>
                                {instructor.name?.toUpperCase() ?? "Unknown"}
                            </h1>
                            <div style={{ marginTop: 4, display: "flex", flexWrap: "wrap", gap: 4 }}>
                                {instructor.instructor_id && (
                                    <span style={{
                                        background: "rgba(255,255,255,0.12)", borderRadius: 4,
                                        padding: "1px 6px", fontSize: 9, fontWeight: 700,
                                        color: "#cbd5e1", textTransform: "uppercase",
                                    }}>
                                        ID-{instructor.instructor_id}
                                    </span>
                                )}
                                {instructor.user_id && (
                                    <span style={{
                                        background: "rgba(99,102,241,0.25)", borderRadius: 4,
                                        padding: "1px 6px", fontSize: 9, fontWeight: 700,
                                        color: "#a5b4fc", textTransform: "uppercase",
                                        display: "flex", alignItems: "center", gap: 3,
                                    }}>
                                        <Layers style={{ width: 8, height: 8, flexShrink: 0 }} />
                                        UID {instructor.user_id}
                                    </span>
                                )}
                            </div>
                        </div>

                        {/* course count — fixed size, never wraps */}
                        <div style={{
                            flexShrink: 0,
                            display: "flex", flexDirection: "column", alignItems: "center",
                            background: "rgba(255,255,255,0.15)", borderRadius: 10,
                            padding: "8px 12px",
                        }}>
                            <span style={{ fontSize: 20, fontWeight: 900, color: "white", lineHeight: 1 }}>
                                {enrolledCount}
                            </span>
                            <span style={{ fontSize: 9, fontWeight: 700, color: "rgba(255,255,255,0.7)", textTransform: "uppercase", marginTop: 2 }}>
                                Courses
                            </span>
                        </div>
                    </div>
                </div>

                {/* ── CONTROLS ── */}
                <div style={{ display: "flex", flexDirection: "column", gap: 8, width: "100%", boxSizing: "border-box" }}>
                    {/* row 1: icon + title */}
                    <div style={{ display: "flex", alignItems: "center", gap: 8, minWidth: 0 }}>
                        <div style={{
                            flexShrink: 0, width: 28, height: 28, borderRadius: 8,
                            background: "#4f46e5",
                            display: "flex", alignItems: "center", justifyContent: "center",
                        }}>
                            <LayoutGrid style={{ width: 14, height: 14, color: "white" }} />
                        </div>
                        <h2 style={{ fontSize: 13, fontWeight: 800, color: "#1e293b", margin: 0, overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap" }}>
                            Weekly Timetable
                        </h2>
                        <span style={{
                            flexShrink: 0, borderRadius: 999, background: "#f1f5f9",
                            padding: "2px 8px", fontSize: 9, fontWeight: 700, color: "#64748b", textTransform: "uppercase",
                        }}>
                            7-day
                        </span>
                    </div>

                    {/* row 2: search (flex-1) + sync */}
                    <div style={{ display: "flex", alignItems: "center", gap: 8, width: "100%", boxSizing: "border-box" }}>
                        <div style={{ flex: 1, minWidth: 0, position: "relative" }}>
                            <Search style={{
                                position: "absolute", left: 10, top: "50%", transform: "translateY(-50%)",
                                width: 14, height: 14, color: "#94a3b8", pointerEvents: "none",
                            }} />
                            <input
                                type="text"
                                placeholder="Search course or room…"
                                value={searchQuery}
                                onChange={e => setSearchQuery(e.target.value)}
                                style={{
                                    width: "100%", boxSizing: "border-box",
                                    paddingLeft: 32, paddingRight: 10, paddingTop: 8, paddingBottom: 8,
                                    fontSize: 12, borderRadius: 10,
                                    border: "1px solid #e2e8f0", background: "white", outline: "none",
                                }}
                            />
                        </div>
                        <button
                            onClick={handleRefresh}
                            disabled={isRefreshing}
                            style={{
                                flexShrink: 0, display: "flex", alignItems: "center", gap: 6,
                                border: "1px solid #e2e8f0", background: "white", borderRadius: 10,
                                padding: "8px 12px", fontSize: 12, fontWeight: 700, color: "#475569",
                                cursor: "pointer", opacity: isRefreshing ? 0.5 : 1,
                            }}
                        >
                            {isRefreshing
                                ? <Loader2 style={{ width: 14, height: 14, animation: "spin 1s linear infinite" }} />
                                : <RefreshCw style={{ width: 14, height: 14 }} />}
                            Sync
                        </button>
                    </div>
                </div>

                {/* ── DESKTOP TABLE (lg+) ── */}
                <div className="hidden lg:block" style={{ borderRadius: 16, border: "1px solid #e2e8f0", background: "white", overflowX: "auto" }}>
                    <table style={{ width: "100%", borderCollapse: "collapse", tableLayout: "fixed", minWidth: 900 }}>
                        <thead>
                            <tr style={{ background: "#f8fafc", borderBottom: "1px solid #f1f5f9" }}>
                                <th style={{ width: 96, padding: 8, borderRight: "1px solid #f1f5f9", fontSize: 9, fontWeight: 700, color: "#94a3b8", textTransform: "uppercase", letterSpacing: "0.1em" }}>
                                    Day / Time
                                </th>
                                {HOURS.map(hour => (
                                    <th key={hour} style={{ padding: 8, borderRight: "1px solid #f8fafc", fontSize: 9, fontWeight: 700, color: "#334155", textTransform: "uppercase" }}>
                                        <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 2 }}>
                                            <Clock style={{ width: 12, height: 12, color: "#818cf8" }} />
                                            {hour}
                                        </div>
                                    </th>
                                ))}
                            </tr>
                        </thead>
                        <tbody>
                            {DAYS.map((day, dayIdx) => (
                                <tr key={day} style={{ borderBottom: "1px solid #f1f5f9" }}>
                                    <td style={{ padding: 8, borderRight: "1px solid #f1f5f9", background: "white" }}>
                                        <span style={{ fontSize: 10, fontWeight: 800, color: "#1e293b" }}>{day}</span>
                                    </td>
                                    {HOURS.map((_, hourIdx) => {
                                        const { entry, isMatch, id, color } = getEntryAt(dayIdx, hourIdx)
                                        return (
                                            <td key={id} style={{
                                                padding: 4, borderRight: "1px solid #f8fafc", height: 112,
                                                opacity: isMatch ? 1 : 0.1, filter: isMatch ? "none" : "grayscale(1)",
                                            }}>
                                                {entry && color ? (
                                                    <div className={`h-full rounded-xl p-2 border ${color.border} ${color.bg} flex flex-col justify-between overflow-hidden`}>
                                                        <h3 className={`text-[9px] font-black leading-tight line-clamp-2 uppercase ${color.text}`}>
                                                            {entry.course.course_name}
                                                        </h3>
                                                        <div style={{ borderTop: "1px solid #f1f5f9", marginTop: 4, paddingTop: 4 }}>
                                                            <div style={{ display: "flex", alignItems: "center", gap: 3, fontSize: 7, color: "#64748b", overflow: "hidden" }}>
                                                                <User style={{ width: 8, height: 8, flexShrink: 0 }} />
                                                                <span style={{ overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap" }}>
                                                                    {entry.course.instructors?.[0]?.name ?? "Staff"}
                                                                </span>
                                                            </div>
                                                            <div className={`flex items-center gap-0.5 text-[7px] font-bold mt-0.5 ${color.accent}`}>
                                                                <MapPin style={{ width: 8, height: 8, flexShrink: 0 }} />
                                                                R-{entry.room.room_id}
                                                            </div>
                                                        </div>
                                                    </div>
                                                ) : (
                                                    <div style={{ height: "100%", border: "1px dashed #f1f5f9", borderRadius: 10, display: "flex", alignItems: "center", justifyContent: "center" }}>
                                                        <span style={{ fontSize: 7, fontFamily: "monospace", color: "#e2e8f0" }}>—</span>
                                                    </div>
                                                )}
                                            </td>
                                        )
                                    })}
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </div>

                {/* ── MOBILE VIEW (below lg) ── */}
                <div className="lg:hidden" style={{
                    width: "100%", boxSizing: "border-box",
                    border: "1px solid #e2e8f0", borderRadius: 16,
                    background: "white", overflow: "hidden",
                }}>
                    {/* Day navigator — arrow + day name + dots. NO pills. */}
                    <div style={{
                        display: "flex", alignItems: "center", justifyContent: "space-between",
                        padding: "8px 12px", background: "#eef2ff",
                        borderBottom: "1px solid #e0e7ff", boxSizing: "border-box", width: "100%",
                    }}>
                        <button
                            onClick={prevDay}
                            aria-label="Previous day"
                            style={{
                                flexShrink: 0, width: 32, height: 32, border: "1px solid #c7d2fe",
                                borderRadius: 8, background: "white", color: "#4f46e5",
                                display: "flex", alignItems: "center", justifyContent: "center", cursor: "pointer",
                            }}
                        >
                            <ChevronLeft style={{ width: 16, height: 16 }} />
                        </button>

                        <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 4, flex: 1, minWidth: 0 }}>
                            <span style={{
                                display: "flex", alignItems: "center", gap: 6,
                                fontSize: 12, fontWeight: 800, color: "#3730a3",
                                textTransform: "uppercase", letterSpacing: "0.05em",
                            }}>
                                <Calendar style={{ width: 13, height: 13, color: "#6366f1", flexShrink: 0 }} />
                                {DAYS[activeDayMobile]}
                            </span>
                            {/* dots — max total width: 7×6 + 6×4 (gaps) = 66px — safe on 320px */}
                            <div style={{ display: "flex", alignItems: "center", gap: 4 }}>
                                {DAYS.map((_, i) => (
                                    <button
                                        key={i}
                                        onClick={() => setActiveDayMobile(i)}
                                        aria-label={DAYS[i]}
                                        style={{
                                            width: i === activeDayMobile ? 16 : 6,
                                            height: 6, borderRadius: 999,
                                            background: i === activeDayMobile ? "#4f46e5" : "#c7d2fe",
                                            border: "none", padding: 0, cursor: "pointer",
                                            transition: "width 0.15s ease",
                                            flexShrink: 0,
                                        }}
                                    />
                                ))}
                            </div>
                        </div>

                        <button
                            onClick={nextDay}
                            aria-label="Next day"
                            style={{
                                flexShrink: 0, width: 32, height: 32, border: "1px solid #c7d2fe",
                                borderRadius: 8, background: "white", color: "#4f46e5",
                                display: "flex", alignItems: "center", justifyContent: "center", cursor: "pointer",
                            }}
                        >
                            <ChevronRight style={{ width: 16, height: 16 }} />
                        </button>
                    </div>

                    {/* Time slots */}
                    <div>
                        {HOURS.map((hour, hourIdx) => {
                            const { entry, isMatch, id, color } = getEntryAt(activeDayMobile, hourIdx)
                            if (searchQuery && !isMatch) return null

                            return (
                                <div key={id} style={{
                                    display: "flex", width: "100%", boxSizing: "border-box",
                                    borderBottom: "1px solid #f1f5f9",
                                }}>
                                    {/* time column — hard fixed width, never grows */}
                                    <div style={{
                                        flexShrink: 0, width: 52, boxSizing: "border-box",
                                        borderRight: "1px solid #f1f5f9", background: "#f8fafc",
                                        display: "flex", flexDirection: "column",
                                        alignItems: "center", justifyContent: "center",
                                        padding: "10px 0", gap: 3,
                                    }}>
                                        <Clock style={{ width: 11, height: 11, color: "#818cf8" }} />
                                        <span style={{ fontSize: 9, fontWeight: 700, color: "#475569", textAlign: "center", lineHeight: 1.3 }}>
                                            {/* split "8:00 AM" → "8:00" + "AM" on two lines */}
                                            {hour.replace(" ", "\n")}
                                        </span>
                                    </div>

                                    {/* card area — flex:1 + minWidth:0 is the key constraint */}
                                    <div style={{ flex: 1, minWidth: 0, padding: 8, boxSizing: "border-box" }}>
                                        {entry && color ? (
                                            <div className={`rounded-xl border ${color.border} ${color.bg} p-2`} style={{ width: "100%", boxSizing: "border-box" }}>
                                                {/* course + id */}
                                                <div style={{ display: "flex", alignItems: "flex-start", gap: 6, marginBottom: 6, minWidth: 0 }}>
                                                    <h3 className={`text-xs font-black leading-snug line-clamp-2 uppercase ${color.text}`}
                                                        style={{ flex: 1, minWidth: 0, margin: 0 }}>
                                                        {entry.course.course_name}
                                                    </h3>
                                                    <span className={`text-[9px] font-bold ${color.accent}`} style={{ flexShrink: 0 }}>
                                                        #{entry.course.course_id}
                                                    </span>
                                                </div>
                                                {/* instructor */}
                                                <div style={{ display: "flex", alignItems: "center", gap: 5, marginBottom: 3, minWidth: 0 }}>
                                                    <User style={{ width: 11, height: 11, flexShrink: 0, color: "#94a3b8" }} />
                                                    <span style={{ fontSize: 11, color: "#64748b", overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap", minWidth: 0 }}>
                                                        {entry.course.instructors?.[0]?.name ?? "Staff"}
                                                    </span>
                                                </div>
                                                {/* room */}
                                                <div style={{ display: "flex", alignItems: "center", gap: 5 }}>
                                                    <MapPin style={{ width: 11, height: 11, flexShrink: 0, color: "#10b981" }} />
                                                    <span className={`text-[11px] font-bold ${color.accent}`}>
                                                        Room {entry.room.room_id}
                                                    </span>
                                                </div>
                                            </div>
                                        ) : (
                                            <div style={{
                                                height: 52, display: "flex", alignItems: "center", justifyContent: "center",
                                                border: "1px dashed #e2e8f0", borderRadius: 10, background: "#f8fafc",
                                            }}>
                                                <span style={{ fontSize: 10, color: "#cbd5e1", fontWeight: 500, textTransform: "uppercase" }}>Free</span>
                                            </div>
                                        )}
                                    </div>
                                </div>
                            )
                        })}
                    </div>
                </div>

                {/* ── FOOTER ── */}
                <div style={{
                    display: "flex", alignItems: "center", justifyContent: "center", gap: 8,
                    border: "1px solid #f1f5f9", borderRadius: 12, background: "#f8fafc",
                    padding: "10px 12px", boxSizing: "border-box", width: "100%",
                }}>
                    <BookOpen style={{ width: 12, height: 12, color: "#818cf8", flexShrink: 0 }} />
                    <p style={{ fontSize: 10, fontWeight: 700, color: "#94a3b8", textTransform: "uppercase", margin: 0, textAlign: "center" }}>
                        {enrolledCount} course{enrolledCount !== 1 ? "s" : ""} · {totalSlots} session{totalSlots !== 1 ? "s" : ""}
                    </p>
                </div>

            </div>
        </div>
    )
}