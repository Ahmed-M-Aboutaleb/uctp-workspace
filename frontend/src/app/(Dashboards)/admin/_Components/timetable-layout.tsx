"use client"

import { useState } from "react"
import {
    Calendar,
    Clock,
    MapPin,
    User,
    RefreshCw,
    Search,
    LayoutGrid,
    Wand2,
    Loader2,
    Info,
    Users,
    ChevronLeft,
    ChevronRight,
} from "lucide-react"
import { TimetableResponse } from "@/Types/TimetableTypes"
import { toast } from "react-toastify"
import { GetTimetable } from "@/ServerActions/Timetable/GetTimetable"
import { TriggerGeneration } from "@/ServerActions/Timetable/TriggerGeneration"

interface TimetableLayoutProps {
    initialTimetable: TimetableResponse
}

const DAYS = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
const DAYS_SHORT = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
const HOURS = [
    "8:00 AM", "9:00 AM", "10:00 AM", "11:00 AM",
    "12:00 PM", "1:00 PM", "2:00 PM", "3:00 PM", "4:00 PM",
]

export default function TimetableLayout({ initialTimetable }: TimetableLayoutProps) {
    const [timetable, setTimetable] = useState<TimetableResponse>(initialTimetable)
    const [isRefreshing, setIsRefreshing] = useState(false)
    const [isGenerating, setIsGenerating] = useState(false)
    const [searchQuery, setSearchQuery] = useState("")
    const [activeDay, setActiveDay] = useState(0)

    const handleRefresh = async () => {
        setIsRefreshing(true)
        try {
            const fresh = await GetTimetable()
            setTimetable(fresh)
        } catch {
            toast.error("Failed to refresh schedule")
        } finally {
            setIsRefreshing(false)
        }
    }

    const handleGenerate = async () => {
        setIsGenerating(true)
        try {
            const result = await TriggerGeneration()
            toast.success(result.message || "Generation started!")
            await handleRefresh()
        } catch {
            toast.error("Failed to trigger generation")
        } finally {
            setIsGenerating(false)
        }
    }

    const getEntryAt = (dayIdx: number, hourIdx: number) => {
        const timeslotId = dayIdx * 9 + hourIdx
        const entry = timetable.find(item => item.timeslot_id === timeslotId) ?? null
        const isMatch =
            !searchQuery ||
            !!entry?.course.course_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
            !!entry?.room.room_id.toString().includes(searchQuery)
        return { entry, isMatch, id: timeslotId }
    }

    const prevDay = () => setActiveDay(d => (d - 1 + DAYS.length) % DAYS.length)
    const nextDay = () => setActiveDay(d => (d + 1) % DAYS.length)

    return (
        /* Root: hard-clip overflow at the very top level */
        <div style={{ width: "100%", maxWidth: "100%", overflowX: "hidden", boxSizing: "border-box" }}>
            <div className="max-w-[1600px] mx-auto space-y-4 p-3 sm:p-4 md:p-6">

                {/* ── HEADER ── */}
                <div className="flex flex-col gap-3 pb-4 border-b border-slate-100">

                    {/* Title */}
                    <div className="flex items-center gap-3">
                        <div className="shrink-0 flex h-10 w-10 items-center justify-center rounded-xl bg-indigo-600">
                            <LayoutGrid className="h-5 w-5 text-white" />
                        </div>
                        <h1 className="text-lg font-extrabold tracking-tight text-slate-900 leading-tight">
                            Academic Timetable
                        </h1>
                    </div>

                    {/* Search */}
                    <div className="relative">
                        <Search className="pointer-events-none absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                        <input
                            type="text"
                            placeholder="Filter by course or room…"
                            value={searchQuery}
                            onChange={e => setSearchQuery(e.target.value)}
                            className="w-full pl-9 pr-3 py-2 text-sm rounded-xl border border-slate-200 outline-none focus:ring-2 focus:ring-indigo-400/30"
                        />
                    </div>

                    {/* Action buttons */}
                    <div className="grid grid-cols-2 gap-2">
                        <button
                            onClick={handleRefresh}
                            disabled={isRefreshing || isGenerating}
                            className="flex items-center justify-center gap-1.5 rounded-xl border border-slate-200 bg-white px-3 py-2.5 text-xs font-bold text-slate-600 active:scale-95 disabled:opacity-50"
                        >
                            <RefreshCw className={`h-3.5 w-3.5 shrink-0 ${isRefreshing ? "animate-spin" : ""}`} />
                            Sync
                        </button>
                        <button
                            onClick={handleGenerate}
                            disabled={isGenerating || isRefreshing}
                            className="flex items-center justify-center gap-1.5 rounded-xl bg-indigo-600 px-3 py-2.5 text-xs font-bold text-white active:scale-95 disabled:opacity-70"
                        >
                            {isGenerating
                                ? <Loader2 className="h-3.5 w-3.5 shrink-0 animate-spin" />
                                : <Wand2 className="h-3.5 w-3.5 shrink-0" />}
                            {isGenerating ? "Generating…" : "Generate"}
                        </button>
                    </div>
                </div>

                {/* ── DESKTOP TABLE (lg+) ── */}
                <div className="hidden lg:block rounded-2xl border border-slate-200 bg-white overflow-x-auto">
                    <table className="w-full border-collapse table-fixed min-w-[1000px]">
                        <thead>
                            <tr className="bg-slate-50">
                                <th className="w-24 p-2 border-b border-r border-slate-100 text-[9px] font-bold uppercase tracking-widest text-slate-400">
                                    Day / Time
                                </th>
                                {HOURS.map(h => (
                                    <th key={h} className="p-2 border-b border-slate-100 text-[9px] font-bold uppercase text-slate-700">
                                        <div className="flex flex-col items-center gap-0.5">
                                            <Clock className="h-3 w-3 text-indigo-500" />
                                            {h}
                                        </div>
                                    </th>
                                ))}
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-slate-100">
                            {DAYS.map((day, dayIdx) => (
                                <tr key={day} className="group hover:bg-slate-50/40">
                                    <td className="p-2 border-r border-slate-100 bg-white group-hover:bg-slate-50">
                                        <span className="text-[10px] font-extrabold text-slate-800">{day}</span>
                                    </td>
                                    {HOURS.map((_, hourIdx) => {
                                        const { entry, isMatch, id } = getEntryAt(dayIdx, hourIdx)
                                        return (
                                            <td
                                                key={id}
                                                className={`p-1 border-r border-slate-50 h-28 transition-opacity ${
                                                    isMatch ? "opacity-100" : "opacity-10 grayscale"
                                                }`}
                                            >
                                                {entry ? (
                                                    <div className="h-full rounded-lg p-1.5 border border-slate-100 bg-white flex flex-col justify-between overflow-hidden">
                                                        <h3 className="text-[9px] font-black text-slate-800 leading-tight line-clamp-2 uppercase">
                                                            {entry.course.course_name}
                                                        </h3>
                                                        <div className="mt-1 pt-1 border-t border-slate-50 space-y-0.5">
                                                            <div className="flex items-center gap-1 text-[8px] text-slate-500 truncate">
                                                                <User className="h-2 w-2 shrink-0" />
                                                                {entry.course.instructors[0]?.name || "Staff"}
                                                            </div>
                                                            <div className="flex items-center justify-between">
                                                                <span className="flex items-center gap-0.5 text-[8px] font-bold text-emerald-600">
                                                                    <MapPin className="h-2 w-2" />
                                                                    R-{entry.room.room_id}
                                                                </span>
                                                                <span className="flex items-center gap-0.5 text-[8px] font-bold text-indigo-600 bg-indigo-50 px-1 py-0.5 rounded">
                                                                    <Users className="h-2 w-2" />
                                                                    {entry.course.students.length}
                                                                </span>
                                                            </div>
                                                        </div>
                                                    </div>
                                                ) : (
                                                    <div className="h-full rounded-lg border border-dashed border-slate-100 flex items-center justify-center">
                                                        <span className="text-[7px] font-mono text-slate-200">FREE</span>
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
                <div className="lg:hidden rounded-2xl border border-slate-200 bg-white overflow-hidden">

                    {/* Day navigator bar */}
                    <div className="flex items-center gap-1.5 px-2 py-2 bg-indigo-50 border-b border-indigo-100">
                        {/* Prev arrow */}
                        <button
                            onClick={prevDay}
                            aria-label="Previous day"
                            className="shrink-0 flex items-center justify-center h-8 w-8 rounded-lg border border-indigo-200 bg-white text-indigo-600 active:scale-95"
                        >
                            <ChevronLeft className="h-4 w-4" />
                        </button>

                        {/* Day pills — flex children with equal basis so they fill the row */}
                        <div className="flex flex-1 gap-1 min-w-0">
                            {DAYS_SHORT.map((d, idx) => (
                                <button
                                    key={d}
                                    onClick={() => setActiveDay(idx)}
                                    className={`
                                        flex-1 min-w-0 h-8 rounded-lg text-[10px] font-bold transition-all truncate
                                        ${activeDay === idx
                                            ? "bg-indigo-600 text-white"
                                            : "bg-white border border-slate-200 text-slate-500"
                                        }
                                    `}
                                >
                                    {d}
                                </button>
                            ))}
                        </div>

                        {/* Next arrow */}
                        <button
                            onClick={nextDay}
                            aria-label="Next day"
                            className="shrink-0 flex items-center justify-center h-8 w-8 rounded-lg border border-indigo-200 bg-white text-indigo-600 active:scale-95"
                        >
                            <ChevronRight className="h-4 w-4" />
                        </button>
                    </div>

                    {/* Active day label */}
                    <div className="flex items-center justify-between px-4 py-2 border-b border-slate-100">
                        <span className="flex items-center gap-2 text-xs font-black text-slate-700 uppercase tracking-wide">
                            <Calendar className="h-3.5 w-3.5 text-indigo-500 shrink-0" />
                            {DAYS[activeDay]}
                        </span>
                        <span className="text-[10px] font-bold text-slate-400">
                            Cycle {activeDay + 1}
                        </span>
                    </div>

                    {/* Time slot list */}
                    <div className="divide-y divide-slate-100">
                        {HOURS.map((hour, hourIdx) => {
                            const { entry, isMatch } = getEntryAt(activeDay, hourIdx)
                            if (searchQuery && !isMatch) return null

                            return (
                                <div key={hourIdx} className="flex min-w-0">

                                    {/* Time column — inline style width = immune to Tailwind purge issues */}
                                    <div
                                        className="shrink-0 flex flex-col items-center justify-center border-r border-slate-100 bg-slate-50/60 py-3 gap-1"
                                        style={{ width: 58 }}
                                    >
                                        <Clock className="h-3 w-3 text-indigo-400" />
                                        <span className="text-[10px] font-bold text-slate-600 text-center leading-tight px-1">
                                            {hour}
                                        </span>
                                    </div>

                                    {/* Content — flex-1 min-w-0 is the key: prevents overflow */}
                                    <div className="flex-1 min-w-0 p-2">
                                        {entry ? (
                                            <div className="rounded-xl border border-slate-200 bg-white p-2.5 min-w-0">

                                                {/* Course name + student count */}
                                                <div className="flex items-start gap-2 min-w-0 mb-2">
                                                    <h3 className="flex-1 min-w-0 text-xs font-black text-slate-900 leading-snug line-clamp-2">
                                                        {entry.course.course_name}
                                                    </h3>
                                                    <span className="shrink-0 flex items-center gap-0.5 text-[10px] font-bold text-indigo-700 bg-indigo-50 px-1.5 py-0.5 rounded-md leading-none">
                                                        <Users className="h-2.5 w-2.5" />
                                                        {entry.course.students.length}
                                                    </span>
                                                </div>

                                                {/* Instructor */}
                                                <div className="flex items-center gap-1.5 min-w-0 mb-1">
                                                    <User className="h-3 w-3 shrink-0 text-indigo-400" />
                                                    <span className="text-[11px] text-slate-500 truncate min-w-0">
                                                        {entry.course.instructors[0]?.name || "Staff"}
                                                    </span>
                                                </div>

                                                {/* Room */}
                                                <div className="flex items-center gap-1.5">
                                                    <MapPin className="h-3 w-3 shrink-0 text-emerald-500" />
                                                    <span className="text-[11px] font-bold text-emerald-600">
                                                        Room {entry.room.room_id}
                                                    </span>
                                                </div>

                                            </div>
                                        ) : (
                                            <div className="h-14 flex items-center justify-center rounded-xl border border-dashed border-slate-100 bg-slate-50/40">
                                                <span className="text-[11px] font-medium text-slate-300 uppercase tracking-widest">
                                                    Free
                                                </span>
                                            </div>
                                        )}
                                    </div>

                                </div>
                            )
                        })}
                    </div>
                </div>

                {/* ── FOOTER ── */}
                <div className="flex items-center justify-center gap-2 rounded-xl border border-slate-100 bg-slate-50 p-3">
                    <Info className="h-3 w-3 shrink-0 text-indigo-400" />
                    <p className="text-[10px] font-bold uppercase tracking-wide text-slate-500 text-center">
                        8-day rotation cycle · Timeslots 0–71
                    </p>
                </div>

            </div>
        </div>
    )
}