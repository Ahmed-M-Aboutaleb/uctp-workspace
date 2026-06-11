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

const DAYS = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"]
const DAYS_SHORT = ["Mon", "Tue", "Wed", "Thu", "Fri"]
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
            toast.success("Generation started!")
            await handleRefresh()
        } catch {
            toast.error("Failed to trigger generation")
        } finally {
            setIsGenerating(false)
        }
    }

    // timeslot_id = dayIdx * 9 + hourIdx  (5 days × 9 hours = 45 slots, 0–44)
    const getEntriesAt = (dayIdx: number, hourIdx: number) => {
        const timeslotId = dayIdx * 9 + hourIdx
        const entries = timetable.filter(item => item.timeslot_id === timeslotId)
        const filtered = searchQuery
            ? entries.filter(
                e =>
                    e.course.course_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
                    e.room.room_id.toString().includes(searchQuery)
            )
            : entries
        return { entries: filtered, timeslotId }
    }

    const prevDay = () => setActiveDay(d => (d - 1 + DAYS.length) % DAYS.length)
    const nextDay = () => setActiveDay(d => (d + 1) % DAYS.length)

    const CourseCard = ({
        entry,
        compact = false,
    }: {
        entry: TimetableResponse[number]
        compact?: boolean
    }) => {
        const border = "border-slate-200"
        const accent = "bg-slate-50"

        return (
            <div
                className={`
                    rounded-lg border-2 ${border} ${accent}
                    flex flex-col justify-between overflow-hidden
                    ${compact ? "p-1.5" : "p-2.5"}
                `}
            >
                <h3
                    className={`
                        font-black text-slate-800 leading-tight uppercase line-clamp-2
                        ${compact ? "text-[9px]" : "text-[11px]"}
                    `}
                >
                    {entry.course.course_name}
                </h3>

                <div className="mt-1 pt-1 border-t border-slate-100 space-y-0.5">
                    <div className={`flex items-center gap-1 text-slate-500 truncate ${compact ? "text-[8px]" : "text-[10px]"}`}>
                        <User className={`shrink-0 ${compact ? "h-2 w-2" : "h-2.5 w-2.5"}`} />
                        {entry.course.instructors[0]?.name || "Staff"}
                    </div>
                    <div className="flex items-center justify-between gap-1">
                        <span className={`flex items-center gap-0.5 font-bold text-emerald-600 ${compact ? "text-[8px]" : "text-[10px]"}`}>
                            <MapPin className={`shrink-0 ${compact ? "h-2 w-2" : "h-2.5 w-2.5"}`} />
                            R-{entry.room.room_id}
                        </span>
                        <span className={`flex items-center gap-0.5 font-bold text-indigo-600 bg-indigo-50 rounded px-1 py-0.5 ${compact ? "text-[8px]" : "text-[10px]"}`}>
                            <Users className={`shrink-0 ${compact ? "h-2 w-2" : "h-2.5 w-2.5"}`} />
                            {entry.course.students.length}
                        </span>
                    </div>
                </div>
            </div>
        )
    }

    return (
        <div style={{ width: "100%", maxWidth: "100%", overflowX: "hidden", boxSizing: "border-box" }}>
            <div className="max-w-[1600px] mx-auto space-y-4 p-3 sm:p-4 md:p-6">

                {/* ── HEADER ── */}
                <div className="flex flex-col gap-3 pb-4 border-b border-slate-100">

                    <div className="flex items-center gap-3">
                        <div className="shrink-0 flex h-10 w-10 items-center justify-center rounded-xl bg-indigo-600">
                            <LayoutGrid className="h-5 w-5 text-white" />
                        </div>
                        <h1 className="text-lg font-extrabold tracking-tight text-slate-900 leading-tight">
                            Academic Timetable
                        </h1>
                    </div>

                    {/* Priority legend */}
                    <div className="flex items-center gap-3 text-[10px] font-bold text-slate-500">
                        <span className="flex items-center gap-1">
                            <span className="inline-block h-3 w-3 rounded border-2 border-emerald-400 bg-emerald-50" />
                            High priority
                        </span>
                        <span className="flex items-center gap-1">
                            <span className="inline-block h-3 w-3 rounded border-2 border-red-400 bg-red-50" />
                            Low priority
                        </span>
                        <span className="flex items-center gap-1">
                            <span className="inline-block h-3 w-3 rounded border-2 border-slate-200 bg-white" />
                            Neutral
                        </span>
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
                    <table className="w-full border-collapse table-fixed min-w-[900px]">
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
                                        const { entries, timeslotId } = getEntriesAt(dayIdx, hourIdx)
                                        return (
                                            <td
                                                key={timeslotId}
                                                className="p-1 border-r border-slate-50 align-top"
                                            >
                                                {entries.length > 0 ? (
                                                    <div className="flex flex-col gap-1 min-h-[7rem]">
                                                        {entries.map(entry => (
                                                            <CourseCard key={entry.id} entry={entry} compact />
                                                        ))}
                                                    </div>
                                                ) : (
                                                    <div className="h-28 rounded-lg border border-dashed border-slate-100 flex items-center justify-center">
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

                    {/* Day navigator */}
                    <div className="flex items-center gap-1.5 px-2 py-2 bg-indigo-50 border-b border-indigo-100">
                        <button
                            onClick={prevDay}
                            aria-label="Previous day"
                            className="shrink-0 flex items-center justify-center h-8 w-8 rounded-lg border border-indigo-200 bg-white text-indigo-600 active:scale-95"
                        >
                            <ChevronLeft className="h-4 w-4" />
                        </button>
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
                            Day {activeDay + 1} of 5
                        </span>
                    </div>

                    {/* Slots */}
                    <div className="divide-y divide-slate-100">
                        {HOURS.map((hour, hourIdx) => {
                            const { entries } = getEntriesAt(activeDay, hourIdx)
                            if (searchQuery && entries.length === 0) return null
                            return (
                                <div key={hourIdx} className="flex min-w-0">
                                    <div
                                        className="shrink-0 flex flex-col items-center justify-start border-r border-slate-100 bg-slate-50/60 py-3 gap-1"
                                        style={{ width: 58 }}
                                    >
                                        <Clock className="h-3 w-3 text-indigo-400" />
                                        <span className="text-[10px] font-bold text-slate-600 text-center leading-tight px-1">
                                            {hour}
                                        </span>
                                    </div>
                                    <div className="flex-1 min-w-0 p-2">
                                        {entries.length > 0 ? (
                                            <div className="flex flex-col gap-2">
                                                {entries.map(entry => (
                                                    <CourseCard key={entry.id} entry={entry} />
                                                ))}
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
                        5-day cycle · Mon–Fri · Timeslots 0–44
                    </p>
                </div>

            </div>
        </div>
    )
}