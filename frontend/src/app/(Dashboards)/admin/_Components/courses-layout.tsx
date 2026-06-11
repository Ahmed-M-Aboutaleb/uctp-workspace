"use client"

import {
    ArrowRight,
    BookMarked,
    BookOpen,
    CheckCircle2,
    GraduationCap,
    Layers,
    Loader2,
    Plus,
    Search,
    User,
    Users
} from "lucide-react"
import { useState } from "react"
import { useForm } from "react-hook-form"
import { toast } from "react-toastify"
import { ActionDropdown } from "@/app/_Components/Shared/ActionDropdown"
import { Modal } from "@/app/_Components/Shared/Modal"

import { CreateCourse } from "@/ServerActions/Course/CreateCourse"
import { DeleteCourse } from "@/ServerActions/Course/DeleteCourse"
import { GetCourses } from "@/ServerActions/Course/GetCourses"
import { UpdateCourse } from "@/ServerActions/Course/UpdateCourse"

import { GetCourseEnrollments } from "@/ServerActions/Enrollment/GetCourseEnrollments"
import { Course, CourseResponse, CreateCourseData } from "@/Types/CourseTypes"
import { StudentResponse } from "@/Types/StudentTypes"
import Link from "next/link"

interface CoursesLayoutProps {
    initialCourses: CourseResponse;
    availableInstructors?: { instructor_id: number; name: string }[];
    availableFeatures?: { feature_id: number; feature_name: string }[];
    availableStudents?: { student_id: number; full_name: string }[];
}

export default function CoursesLayout({
    initialCourses,
    availableInstructors = [],
    availableFeatures = [],
    availableStudents = []
}: CoursesLayoutProps) {
    const [data, setData] = useState<Course[]>(initialCourses)
    const [searchQuery, setSearchQuery] = useState("")
    const [skip, setSkip] = useState(0)
    const [isLoadingMore, setIsLoadingMore] = useState(false)

    const [hasMore, setHasMore] = useState(initialCourses.length === 100)
    const LIMIT = 100
    const [isDialogOpen, setIsDialogOpen] = useState(false)
    const [editingItem, setEditingItem] = useState<Course | null>(null)
    const [isSubmitting, setIsSubmitting] = useState(false)
    const [itemToDelete, setItemToDelete] = useState<number | null>(null)
    const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false)

    const [isEnrollmentsModalOpen, setIsEnrollmentsModalOpen] = useState(false)
    const [enrollmentsData, setEnrollmentsData] = useState<StudentResponse | null>(null)
    const [isLoadingEnrollments, setIsLoadingEnrollments] = useState(false)
    const [selectedCourseName, setSelectedCourseName] = useState("")

    const { register, handleSubmit, reset, formState: { errors } } = useForm<CreateCourseData>()

    const filteredData = data
        .filter(item =>
            item.course_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
            item.course_id.toString().includes(searchQuery)
        )
        .sort((a, b) => a.course_id - b.course_id)

    const handleLoadMore = async () => {
        setIsLoadingMore(true)
        try {
            const nextSkip = skip + LIMIT
            const newCourses = await GetCourses(nextSkip, LIMIT)
            if (newCourses.length < LIMIT) setHasMore(false)
            setData((prev) => [...prev, ...newCourses])
            setSkip(nextSkip)
        } catch (error) {
            toast.error("Failed to load more records")
        } finally {
            setIsLoadingMore(false)
        }
    }

    const handleOpenDialog = (item?: Course) => {
        if (item) {
            setEditingItem(item)
            reset({
                course_name: item.course_name,
                student_ids: item.students?.map(s => s.student_id) || [],
                instructor_ids: item.instructors?.map(i => i.instructor_id) || [],
                feature_ids: item.features?.map(f => f.feature_id) || []
            })
        } else {
            setEditingItem(null)
            reset({
                course_name: "",
                student_ids: [],
                instructor_ids: [],
                feature_ids: []
            })
        }
        setIsDialogOpen(true)
    }

    const handleViewEnrollments = async (course: Course) => {
        setSelectedCourseName(course.course_name)
        setIsEnrollmentsModalOpen(true)
        setEnrollmentsData(null)
        setIsLoadingEnrollments(true)
        try {
            const result = await GetCourseEnrollments(course.course_id)
            setEnrollmentsData(result)
        } catch (error) {
            toast.error("Failed to load enrollments")
            setIsEnrollmentsModalOpen(false)
        } finally {
            setIsLoadingEnrollments(false)
        }
    }

    const onSubmit = async (formData: CreateCourseData) => {
        setIsSubmitting(true)
        try {
            const payload = {
                ...formData,
                student_ids: (formData.student_ids || []).map(Number),
                instructor_ids: (formData.instructor_ids || []).map(Number),
                feature_ids: (formData.feature_ids || []).map(Number)
            }

            if (editingItem) {
                const result = await UpdateCourse(editingItem.course_id, payload)
                setData(prevData => prevData.map(c =>
                    c.course_id === editingItem.course_id ? result : c
                ))
                toast.success("Course updated successfully")
            } else {
                const result = await CreateCourse(payload)
                setData(prev => [result, ...prev])
                toast.success("Course created successfully")
            }
            setIsDialogOpen(false)
            reset()
        } catch (error: any) {
            toast.error(error.message || "An error occurred")
        } finally {
            setIsSubmitting(false)
        }
    }

    const confirmDelete = async () => {
        
        if (!itemToDelete) return
        setIsSubmitting(true)
        try {
            await DeleteCourse(itemToDelete)
            setData(data.filter(c => c.course_id !== itemToDelete))
            toast.success("Course deleted")
            setIsDeleteModalOpen(false)
        } catch (error: any) {
            toast.error("Failed to delete course")
        } finally {
            setIsSubmitting(false)
        }
    }

    return (
        <div className="max-w-6xl mx-auto p-4 md:p-8 space-y-6 md:space-y-8 animate-in fade-in duration-500">

            {/* Header */}
            <div className="flex flex-col gap-6 md:flex-row md:items-center md:justify-between border-b border-slate-100 pb-8">
                <div className="flex items-center gap-4">
                    <div className="flex h-12 w-12 md:h-14 md:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-indigo-500 shadow-xl shadow-indigo-100">
                        <BookMarked className="h-6 w-6 md:h-7 md:w-7 text-white" />
                    </div>
                    <div>
                        <h1 className="text-xl md:text-3xl font-extrabold tracking-tight text-slate-900 leading-tight">Course Catalog</h1>
                        <p className="text-xs md:text-base text-slate-500 font-medium">Manage curriculum and participant enrollment</p>
                    </div>
                </div>
                <button
                    onClick={() => handleOpenDialog()}
                    className="flex w-full md:w-auto items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-indigo-500 px-6 py-3 text-sm font-bold text-white shadow-lg hover:shadow-indigo-200 hover:-translate-y-0.5 transition-all active:scale-95 cursor-pointer"
                >
                    <Plus className="h-5 w-5" />
                    Create Course
                </button>
            </div>

            {/* Table / Cards */}
            <div className="rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">

                {/* Search bar */}
                <div className="p-4 md:p-6 border-b border-slate-50 bg-slate-50/30">
                    <div className="relative w-full md:max-w-sm">
                        <Search className="absolute left-3.5 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
                        <input
                            type="search"
                            placeholder="Search courses..."
                            className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-indigo-500/10 focus:border-indigo-500 outline-none transition-all"
                            value={searchQuery}
                            onChange={(e) => setSearchQuery(e.target.value)}
                        />
                    </div>
                </div>

                {/* ── DESKTOP: table ── */}
                <div className="hidden md:block overflow-x-auto">
                    <table className="w-full text-left">
                        <thead>
                            <tr className="bg-slate-50/50">
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">ID</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Course Name</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Instructors</th>
                                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Enrollment</th>
                                <th className="px-8 py-4 text-right text-xs font-bold uppercase tracking-widest text-slate-400">Actions</th>
                            </tr>
                        </thead>
                        <tbody className="divide-y divide-slate-100">
                            {filteredData.map((course) => (
                                <tr key={course.course_id} className="group hover:bg-indigo-50/20 transition-colors">
                                    <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">
                                        #{course.course_id.toString().padStart(3, '0')}
                                    </td>
                                    <td className="px-8 py-5">
                                        <div className="text-sm font-bold text-slate-800">{course.course_name}</div>
                                        <div className="flex gap-1 mt-1">
                                            {course.features?.map(f => (
                                                <span key={f.feature_id} className="text-[9px] bg-slate-100 text-slate-500 px-1.5 py-0.5 rounded uppercase font-bold">
                                                    {f.feature_name}
                                                </span>
                                            ))}
                                        </div>
                                    </td>
                                    <td className="px-8 py-5">
                                        <div className="flex flex-col gap-1">
                                            {course.instructors && course.instructors.length > 0
                                                ? course.instructors.map(inst => (
                                                    <div key={inst.instructor_id} className="text-xs text-slate-600 flex items-center gap-1.5">
                                                        <User className="h-3 w-3 text-slate-400" /> {inst.name}
                                                    </div>
                                                ))
                                                : <span className="text-xs text-slate-400 italic">Unassigned</span>
                                            }
                                        </div>
                                    </td>
                                    <td className="px-8 py-5">
                                        <Link
                                            href={`enrollments/${course.course_id}`}
                                            className="inline-flex items-center gap-2 px-3 py-1 bg-slate-100 hover:bg-indigo-100 hover:text-indigo-700 rounded-full text-xs font-bold text-slate-600 transition-colors group/badge"
                                        >
                                            <Users className="h-3.5 w-3.5 group-hover/badge:text-indigo-500" />
                                            {course.students?.length || 0} Students
                                        </Link>
                                    </td>
                                    <td className="px-8 py-5 text-right">
                                        <ActionDropdown
                                            onEdit={() => handleOpenDialog(course)}
                                            onDelete={() => { setItemToDelete(course.course_id); setIsDeleteModalOpen(true) }}
                                        />
                                    </td>
                                </tr>
                            ))}
                        </tbody>
                    </table>

                    {filteredData.length === 0 && (
                        <div className="flex flex-col items-center justify-center py-20 gap-3">
                            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-slate-100">
                                <BookMarked className="h-7 w-7 text-slate-300" />
                            </div>
                            <p className="text-sm font-bold text-slate-400">No courses found</p>
                        </div>
                    )}
                </div>

                {/* ── MOBILE: cards ── */}
                <div className="md:hidden divide-y divide-slate-100">
                    {filteredData.length === 0 ? (
                        <div className="flex flex-col items-center justify-center py-20 gap-3">
                            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-slate-100">
                                <BookMarked className="h-7 w-7 text-slate-300" />
                            </div>
                            <p className="text-sm font-bold text-slate-400">No courses found</p>
                        </div>
                    ) : (
                        filteredData.map((course) => (
                            <div key={course.course_id} className="p-4 hover:bg-indigo-50/20 transition-colors">
                                {/* Card top row: ID + name + actions */}
                                <div className="flex items-start justify-between gap-3">
                                    <div className="flex items-center gap-3 min-w-0">
                                        <span className="shrink-0 text-xs font-mono font-bold text-indigo-500 bg-indigo-50 px-2 py-1 rounded-lg">
                                            #{course.course_id.toString().padStart(3, '0')}
                                        </span>
                                        <p className="text-sm font-bold text-slate-800 leading-snug truncate">
                                            {course.course_name}
                                        </p>
                                        <Link
                                            href={`enrollments/${course.course_id}`}
                                            className="inline-flex items-center gap-2 px-3 py-1.5 bg-slate-100 hover:bg-indigo-100 hover:text-indigo-700 rounded-full text-xs font-bold text-slate-600 transition-colors"
                                        >
                                            <Users className="h-3.5 w-3.5" />
                                            {course.students?.length || 0} Students
                                        </Link>
                                    </div>
                                    <ActionDropdown
                                        onEdit={() => handleOpenDialog(course)}
                                        onDelete={() => { setItemToDelete(course.course_id); setIsDeleteModalOpen(true) }}
                                    />
                                </div>

                                {/* Feature tags */}
                                {course.features && course.features.length > 0 && (
                                    <div className="flex flex-wrap gap-1 mt-2 ml-1">
                                        {course.features.map(f => (
                                            <span key={f.feature_id} className="text-[9px] bg-slate-100 text-slate-500 px-1.5 py-0.5 rounded uppercase font-bold">
                                                {f.feature_name}
                                            </span>
                                        ))}
                                    </div>
                                )}

                                {/* Instructors row */}
                                <div className="mt-3 flex flex-wrap gap-x-3 gap-y-1">
                                    {course.instructors && course.instructors.length > 0
                                        ? course.instructors.map(inst => (
                                            <div key={inst.instructor_id} className="flex items-center gap-1 text-xs text-slate-500">
                                                <User className="h-3 w-3 text-slate-400" /> {inst.name}
                                            </div>
                                        ))
                                        : <span className="text-xs text-slate-400 italic">No instructor assigned</span>
                                    }
                                </div>

                                {/* Enrollment button */}
                                <div className="mt-3">
                                    <button
                                        onClick={() => handleViewEnrollments(course)}
                                        className="inline-flex items-center gap-2 px-3 py-1.5 bg-slate-100 hover:bg-indigo-100 hover:text-indigo-700 rounded-full text-xs font-bold text-slate-600 transition-colors"
                                    >
                                        <Users className="h-3.5 w-3.5" />
                                        {course.students?.length || 0} Students
                                    </button>
                                </div>
                            </div>
                        ))
                    )}
                </div>

                {/* Footer */}
                <div className="p-4 md:p-6 bg-slate-50/30 border-t border-slate-100 flex items-center justify-between">
                    <p className="text-sm text-slate-500">
                        Total Courses: <span className="text-slate-900 font-bold">{data.length}</span>
                    </p>
                    {hasMore && (
                        <button
                            onClick={handleLoadMore}
                            disabled={isLoadingMore}
                            className="flex items-center gap-2 rounded-xl border border-slate-200 bg-white px-5 py-2.5 text-sm font-bold text-slate-700 hover:bg-slate-50 transition-all"
                        >
                            {isLoadingMore ? <Loader2 className="h-4 w-4 animate-spin" /> : <>Load More <ArrowRight className="h-4 w-4" /></>}
                        </button>
                    )}
                </div>
            </div>

            {/* ── Enrollments Modal ── */}
            <Modal
                open={isEnrollmentsModalOpen}
                onClose={() => setIsEnrollmentsModalOpen(false)}
                title="Enrolled Students"
                description={`Viewing all students enrolled in "${selectedCourseName}"`}
            >
                <div className="mt-2 min-h-[200px]">
                    {isLoadingEnrollments ? (
                        <div className="flex flex-col items-center justify-center py-16 gap-3">
                            <Loader2 className="h-8 w-8 animate-spin text-indigo-500" />
                            <p className="text-sm text-slate-400 font-medium">Loading students...</p>
                        </div>
                    ) : enrollmentsData && enrollmentsData.length > 0 ? (
                        <div className="space-y-2 max-h-[60vh] overflow-y-auto pr-1 custom-scrollbar">
                            <div className="flex items-center gap-2 mb-4">
                                <span className="inline-flex items-center gap-1.5 px-3 py-1 bg-indigo-50 text-indigo-700 rounded-full text-xs font-bold">
                                    <Users className="h-3.5 w-3.5" />
                                    {enrollmentsData.length} {enrollmentsData.length === 1 ? "Student" : "Students"} Enrolled
                                </span>
                            </div>
                            {enrollmentsData.map((student, index) => (
                                <div
                                    key={student.student_id}
                                    className="flex items-center gap-4 p-3.5 rounded-2xl border border-slate-100 bg-slate-50/50 hover:bg-white hover:border-indigo-100 hover:shadow-sm transition-all"
                                >
                                    <div className="flex h-9 w-9 shrink-0 items-center justify-center rounded-xl bg-gradient-to-br from-indigo-500 to-indigo-600 text-white text-sm font-bold shadow-sm">
                                        {student.full_name?.charAt(0).toUpperCase() ?? "?"}
                                    </div>
                                    <div className="flex-1 min-w-0">
                                        <p className="text-sm font-bold text-slate-800 truncate">{student.full_name}</p>
                                        <span className="inline-flex items-center gap-1 mt-0.5 px-2 py-0.5 rounded-full text-[10px] font-bold uppercase tracking-wide bg-indigo-50 text-indigo-500">
                                            {student.semester}
                                        </span>
                                    </div>
                                    <span className="text-[11px] font-mono text-slate-300 font-bold shrink-0">
                                        #{String(index + 1).padStart(2, "0")}
                                    </span>
                                </div>
                            ))}
                        </div>
                    ) : (
                        <div className="flex flex-col items-center justify-center py-16 gap-3 text-center">
                            <div className="flex h-14 w-14 items-center justify-center rounded-2xl bg-slate-100">
                                <GraduationCap className="h-7 w-7 text-slate-300" />
                            </div>
                            <p className="text-sm font-bold text-slate-500">No students enrolled</p>
                            <p className="text-xs text-slate-400">This course has no enrolled students yet.</p>
                        </div>
                    )}
                </div>
                <div className="flex justify-end pt-6 border-t border-slate-100 mt-4">
                    <button
                        onClick={() => setIsEnrollmentsModalOpen(false)}
                        className="px-6 py-2.5 text-sm font-bold text-slate-500 hover:bg-slate-50 rounded-xl transition-colors"
                    >
                        Close
                    </button>
                </div>
            </Modal>

            {/* ── Create / Edit Modal ── */}
            <Modal
                open={isDialogOpen}
                onClose={() => !isSubmitting && setIsDialogOpen(false)}
                title={editingItem ? "Edit Course" : "Create New Course"}
                description="Update details and participants for this course."
            >
                <form onSubmit={handleSubmit(onSubmit)} className="space-y-6 max-h-[75vh] overflow-y-auto px-1 custom-scrollbar">
                    <div className="space-y-1.5">
                        <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Course Title</label>
                        <div className="relative">
                            <BookOpen className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                            <input
                                {...register("course_name", { required: "Course name is required" })}
                                className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all"
                                placeholder="E.g. Computer Science 101"
                            />
                        </div>
                        {errors.course_name && (
                            <p className="text-xs text-red-500 ml-1">{errors.course_name.message}</p>
                        )}
                    </div>

                    <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                        {/* Instructors */}
                        <div className="space-y-3">
                            <div className="flex items-center gap-2 text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">
                                <User className="h-3.5 w-3.5" /> Assign Instructors
                            </div>
                            <div className="grid grid-cols-1 gap-2 p-3 rounded-2xl bg-slate-50 border border-slate-100 max-h-[200px] overflow-y-auto shadow-inner">
                                {availableInstructors.map((inst) => (
                                    <label key={inst.instructor_id} className="flex items-center gap-3 p-2 rounded-lg hover:bg-white hover:shadow-sm cursor-pointer transition-all group">
                                        <div className="relative flex items-center justify-center">
                                            <input type="checkbox" value={inst.instructor_id} {...register("instructor_ids")} className="peer h-5 w-5 opacity-0 absolute cursor-pointer" />
                                            <div className="h-5 w-5 border-2 border-slate-300 rounded-md peer-checked:border-indigo-600 peer-checked:bg-indigo-600 flex items-center justify-center">
                                                <CheckCircle2 className="h-3.5 w-3.5 text-white scale-0 peer-checked:scale-100 transition-transform" />
                                            </div>
                                        </div>
                                        <span className="text-sm text-slate-600 font-medium">{inst.name}</span>
                                    </label>
                                ))}
                            </div>
                        </div>

                        {/* Students */}
                        <div className="space-y-3">
                            <div className="flex items-center gap-2 text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">
                                <GraduationCap className="h-3.5 w-3.5" /> Enroll Students
                            </div>
                            <div className="grid grid-cols-1 gap-2 p-3 rounded-2xl bg-slate-50 border border-slate-100 max-h-[200px] overflow-y-auto shadow-inner">
                                {availableStudents.map((stud) => (
                                    <label key={stud.student_id} className="flex items-center gap-3 p-2 rounded-lg hover:bg-white hover:shadow-sm cursor-pointer transition-all group">
                                        <div className="relative flex items-center justify-center">
                                            <input type="checkbox" value={stud.student_id} {...register("student_ids")} className="peer h-5 w-5 opacity-0 absolute cursor-pointer" />
                                            <div className="h-5 w-5 border-2 border-slate-300 rounded-md peer-checked:border-indigo-600 peer-checked:bg-indigo-600 flex items-center justify-center">
                                                <CheckCircle2 className="h-3.5 w-3.5 text-white scale-0 peer-checked:scale-100 transition-transform" />
                                            </div>
                                        </div>
                                        <span className="text-sm text-slate-600 font-medium">{stud.full_name}</span>
                                    </label>
                                ))}
                            </div>
                        </div>
                    </div>

                    {/* Features */}
                    <div className="space-y-3">
                        <div className="flex items-center gap-2 text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">
                            <Layers className="h-3.5 w-3.5" /> Features
                        </div>
                        <div className="grid grid-cols-2 sm:grid-cols-3 gap-2 p-3 rounded-2xl bg-slate-50 border border-slate-100">
                            {availableFeatures.map((feat) => (
                                <label key={feat.feature_id} className="flex items-center gap-3 p-2 rounded-lg hover:bg-white hover:shadow-sm cursor-pointer transition-all">
                                    <div className="relative flex items-center justify-center">
                                        <input type="checkbox" value={feat.feature_id} {...register("feature_ids")} className="peer h-5 w-5 opacity-0 absolute cursor-pointer" />
                                        <div className="h-5 w-5 border-2 border-slate-300 rounded-md peer-checked:border-indigo-600 peer-checked:bg-indigo-600 flex items-center justify-center">
                                            <CheckCircle2 className="h-3.5 w-3.5 text-white scale-0 peer-checked:scale-100 transition-transform" />
                                        </div>
                                    </div>
                                    <span className="text-sm text-slate-600 font-medium">{feat.feature_name}</span>
                                </label>
                            ))}
                        </div>
                    </div>

                    <div className="flex justify-end gap-3 pt-6 border-t border-slate-100 sticky bottom-0 bg-white">
                        <button type="button" onClick={() => setIsDialogOpen(false)} className="px-6 py-3 text-sm font-bold text-slate-500 hover:bg-slate-50 rounded-xl transition-colors">
                            Cancel
                        </button>
                        <button type="submit" disabled={isSubmitting} className="flex items-center gap-2 rounded-xl bg-indigo-600 px-8 py-3 text-sm font-bold text-white hover:bg-indigo-700 shadow-lg active:scale-95 disabled:opacity-50 transition-all">
                            {isSubmitting ? <Loader2 className="h-4 w-4 animate-spin" /> : (editingItem ? "Update Course" : "Create Course")}
                        </button>
                    </div>
                </form>
            </Modal>

            {/* ── Delete Modal ── */}
            <Modal
                open={isDeleteModalOpen}
                onClose={() => !isSubmitting && setIsDeleteModalOpen(false)}
                title="Delete Course"
                description="Are you sure you want to delete this course? This action is permanent."
            >
                <div className="flex justify-end gap-3 mt-8">
                    <button onClick={() => setIsDeleteModalOpen(false)} className="px-6 py-3 text-sm font-bold text-slate-500">
                        Cancel
                    </button>
                    <button onClick={confirmDelete} disabled={isSubmitting} className="rounded-xl bg-red-600 px-8 py-3 text-sm font-bold text-white hover:bg-red-700 transition-colors shadow-lg shadow-red-100">
                        {isSubmitting ? "Deleting..." : "Confirm Delete"}
                    </button>
                </div>
            </Modal>
        </div>
    )
}