"use client"

import { useState } from "react"
import { useForm } from "react-hook-form"
import { 
  Plus, Search, GraduationCap, Loader2, ArrowRight, 
  Mail, User, Lock, Hash, ShieldAlert, ChevronRight 
} from "lucide-react"
import { toast } from "react-toastify"
import { Modal } from "@/app/_Components/Shared/Modal"
import { ActionDropdown } from "@/app/_Components/Shared/ActionDropdown"
import { CreateInstructor } from "@/ServerActions/Instructor/CreateInstructor"
import { UpdateInstructor } from "@/ServerActions/Instructor/UpdateInstructor"
import { DeleteInstructor } from "@/ServerActions/Instructor/DeleteInstructor"
import { InstructorResponse, CreateInstructorData } from "@/Types/InstructorTypes"
import { GetInstructors } from "@/ServerActions/Instructor/GetInstructor"

export default function InstructorsLayout({ initialInstructors }: { initialInstructors: InstructorResponse[] }) {
  const [data, setData] = useState<InstructorResponse[]>(initialInstructors)
  const [searchQuery, setSearchQuery] = useState("")
  
  const [skip, setSkip] = useState(0)
  const [isLoadingMore, setIsLoadingMore] = useState(false)
  const [hasMore, setHasMore] = useState(initialInstructors.length === 100)
  const LIMIT = 100

  const [isDialogOpen, setIsDialogOpen] = useState(false)
  const [editingItem, setEditingItem] = useState<InstructorResponse | null>(null)
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [itemToDelete, setItemToDelete] = useState<number | null>(null)
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false)

  const { register, handleSubmit, reset, formState: { errors } } = useForm<CreateInstructorData>()

  const filteredData = data
    .filter(item =>
      item.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      item.instructor_id.toString().includes(searchQuery)
    )
    .sort((a, b) => a.instructor_id - b.instructor_id)

  const handleLoadMore = async () => {
    setIsLoadingMore(true)
    try {
      const nextSkip = skip + LIMIT
      const newInstructors = await GetInstructors(nextSkip, LIMIT)
      if (newInstructors.length < LIMIT) setHasMore(false)
      setData((prev) => [...prev, ...newInstructors])
      setSkip(nextSkip)
    } catch (error) {
      toast.error("Failed to load more records")
    } finally {
      setIsLoadingMore(false)
    }
  }

  const handleOpenDialog = (item?: InstructorResponse) => {
    if (item) {
      setEditingItem(item)
      reset({ 
        name: item.name,
        email: "", 
        password: "" 
      })
    } else {
      setEditingItem(null)
      reset({ 
        name: "", 
        email: "", 
        password: "" 
      })
    }
    setIsDialogOpen(true)
  }

  const onSubmit = async (formData: CreateInstructorData) => {
    setIsSubmitting(true);
    try {
      if (editingItem) {
        const result = await UpdateInstructor(editingItem.instructor_id, formData.name)
        setData(data.map(i => i.instructor_id === editingItem.instructor_id ? result : i))
        toast.success("Instructor updated successfully")
      } else {
        const result = await CreateInstructor(formData)
        setData(prev => [result, ...prev])
        toast.success("Instructor created successfully")
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
      await DeleteInstructor(itemToDelete)
      setData(data.filter(i => i.instructor_id !== itemToDelete))
      toast.success("Instructor removed")
      setIsDeleteModalOpen(false)
    } catch (error: any) {
      toast.error("Failed to delete")
    } finally {
      setIsSubmitting(false)
    }
  }
  
  return (
    <div className="max-w-6xl mx-auto p-4 md:p-8 space-y-6 md:space-y-8 animate-in fade-in duration-500">
      
      {/* Header Section */}
      <div className="flex flex-col gap-6 md:flex-row md:items-center md:justify-between border-b border-slate-100 pb-8">
        <div className="flex items-center gap-4">
          <div className="flex h-12 w-12 md:h-14 md:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-violet-500 shadow-xl shadow-emerald-100">
            <GraduationCap className="h-6 w-6 md:h-7 md:w-7 text-white" />
          </div>
          <div>
            <h1 className="text-xl md:text-3xl font-extrabold tracking-tight text-slate-900 leading-tight">Instructors</h1>
            <p className="text-xs md:text-base text-slate-500 font-medium">Manage faculty members and teaching staff</p>
          </div>
        </div>
        <button
          onClick={() => handleOpenDialog()}
          className="flex w-full md:w-auto items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 px-6 py-3 text-sm font-bold text-white shadow-lg hover:shadow-emerald-200 hover:-translate-y-0.5 transition-all active:scale-95 cursor-pointer"
        >
          <Plus className="h-5 w-5" />
          Add Instructor
        </button>
      </div>

      <div className="rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">
        <div className="p-4 md:p-6 border-b border-slate-50 bg-slate-50/30">
          <div className="relative w-full md:max-w-sm">
            <Search className="absolute left-3.5 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
            <input
              type="search"
              placeholder="Search by name or ID..."
              className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-emerald-500/10 focus:border-emerald-500 outline-none transition-all"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>
        </div>

        <div className="md:hidden divide-y divide-slate-100">
          {filteredData.length === 0 ? (
            <div className="py-12 text-center text-slate-400 italic text-sm">No instructors found.</div>
          ) : (
            filteredData.map((inst) => (
              <div key={inst.instructor_id} className="p-5 flex flex-col gap-4 hover:bg-slate-50/50 transition-colors">
                <div className="flex justify-between items-center">
                  <div className="flex items-center gap-2 text-[10px] font-mono font-bold text-emerald-600 bg-emerald-50 px-2.5 py-1 rounded-lg">
                    <Hash className="h-3 w-3" /> {inst.instructor_id.toString().padStart(3, '0')}
                  </div>
                  <ActionDropdown 
                    onEdit={() => handleOpenDialog(inst)} 
                    onDelete={() => { setItemToDelete(inst.instructor_id); setIsDeleteModalOpen(true); }} 
                  />
                </div>
                <div className="space-y-1">
                  <h3 className="text-sm font-bold text-slate-900">{inst.name}</h3>
                  <p className="text-[10px] text-slate-400 uppercase tracking-tighter">User Reference: {inst.user_id}</p>
                </div>
              </div>
            ))
          )}
        </div>

        <div className="hidden md:block overflow-x-auto">
          <table className="w-full text-left">
            <thead>
              <tr className="bg-slate-50/50">
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">ID</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Instructor Name</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">User ID</th>
                <th className="px-8 py-4 text-right text-xs font-bold uppercase tracking-widest text-slate-400">Actions</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-100">
              {filteredData.map((inst) => (
                <tr key={inst.instructor_id} className="group hover:bg-indigo-50/20 transition-colors">
                  <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">
                    #{inst.instructor_id.toString().padStart(3, '0')}
                  </td>
                  <td className="px-8 py-5">
                    <div className="text-sm font-bold text-slate-800">{inst.name}</div>
                  </td>
                  <td className="px-8 py-5">
                    <span className="text-xs font-medium text-slate-500 bg-slate-100 px-2 py-1 rounded-md">
                      UID: {inst.user_id}
                    </span>
                  </td>
                  <td className="px-8 py-5 text-right">
                    <ActionDropdown 
                      onEdit={() => handleOpenDialog(inst)} 
                      onDelete={() => { setItemToDelete(inst.instructor_id); setIsDeleteModalOpen(true); }} 
                    />
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Footer */}
        <div className="p-5 md:p-6 bg-slate-50/30 border-t border-slate-100 flex flex-col md:flex-row items-center justify-between gap-4">
          <p className="text-sm text-slate-500 font-medium order-2 md:order-1">
            Total Instructors: <span className="text-slate-900 font-bold">{data.length}</span>
          </p>
          {hasMore && (
            <button
              onClick={handleLoadMore}
              disabled={isLoadingMore}
              className="flex w-full md:w-auto items-center justify-center gap-2 rounded-xl border border-slate-200 bg-white px-5 py-2.5 text-sm font-bold text-slate-700 shadow-sm hover:bg-slate-50 transition-all active:scale-95 disabled:opacity-50 cursor-pointer order-1 md:order-2"
            >
              {isLoadingMore ? <Loader2 className="h-4 w-4 animate-spin" /> : <>Load More <ArrowRight className="h-4 w-4" /></>}
            </button>
          )}
        </div>
      </div>

      <Modal
        open={isDialogOpen}
        onClose={() => !isSubmitting && setIsDialogOpen(false)}
        title={editingItem ? "Edit Instructor" : "New Instructor"}
        description={editingItem ? "Update faculty member information." : "Create a new instructor profile and login credentials."}
      >
        <form 
          key={editingItem ? 'edit' : 'create'}
          onSubmit={handleSubmit(onSubmit)} 
          className="space-y-5" 
          autoComplete="off"
        >
          <div className="space-y-1.5">
            <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Full Name</label>
            <div className="relative">
              <User className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
              <input 
                {...register("name", { required: "Name is required" })} 
                className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" 
                placeholder="Instructor Name" 
              />
            </div>
            {errors.name && <p className="text-red-500 text-[10px] mt-1 ml-1 font-semibold">{errors.name.message}</p>}
          </div>

          {!editingItem && (
            <div className="pt-4 space-y-4 border-t border-slate-100">
              <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Email Address</label>
                <div className="relative">
                  <Mail className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                  <input 
                    {...register("email", { required: !editingItem })} 
                    type="email" 
                    autoComplete="off"
                    className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" 
                    placeholder="instructor@university.edu" 
                  />
                </div>
              </div>
              <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Account Password</label>
                <div className="relative">
                  <Lock className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                  <input 
                    {...register("password", { required: !editingItem })} 
                    type="password" 
                    autoComplete="new-password"
                    className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" 
                    placeholder="••••••••" 
                  />
                </div>
              </div>
            </div>
          )}

          <div className="flex flex-col md:flex-row justify-end gap-3 pt-6 border-t border-slate-100">
            <button 
              type="button" 
              disabled={isSubmitting} 
              onClick={() => setIsDialogOpen(false)} 
              className="px-6 py-3 text-sm font-bold text-slate-500 hover:bg-slate-50 rounded-xl transition-colors order-2 md:order-1"
            >
              Cancel
            </button>
            <button 
              type="submit" 
              disabled={isSubmitting} 
              className="flex items-center justify-center gap-2 rounded-xl bg-indigo-600 px-8 py-3 text-sm font-bold text-white hover:bg-indigo-700 shadow-lg shadow-indigo-100 active:scale-95 disabled:opacity-50 transition-all order-1 md:order-2"
            >
              {isSubmitting ? <Loader2 className="h-4 w-4 animate-spin" /> : (editingItem ? "Update Instructor" : "Create Instructor")}
            </button>
          </div>
        </form>
      </Modal>

      {/* Delete Modal */}
      <Modal open={isDeleteModalOpen} onClose={() => !isSubmitting && setIsDeleteModalOpen(false)} title="Confirm Deletion" description="Are you sure you want to remove this instructor? This will revoke their access to the system.">
        <div className="flex flex-col md:flex-row justify-end gap-3 mt-8">
          <button onClick={() => setIsDeleteModalOpen(false)} className="px-6 py-3 text-sm font-bold text-slate-500 order-2 md:order-1">Cancel</button>
          <button onClick={confirmDelete} disabled={isSubmitting} className="rounded-xl bg-red-600 px-8 py-3 text-sm font-bold text-white hover:bg-red-700 transition-colors order-1 md:order-2 shadow-lg shadow-red-100">
            {isSubmitting ? "Deleting..." : "Delete Instructor"}
          </button>
        </div>
      </Modal>
    </div>
  )
}