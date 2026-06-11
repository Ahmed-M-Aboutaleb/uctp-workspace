"use client"

import { useState } from "react"
import { useForm } from "react-hook-form"
import { 
  Plus, Search, ShieldCheck, Loader2, ArrowRight, 
  Mail, Briefcase, Building2, User, Lock, Hash, ShieldAlert, ChevronDown 
} from "lucide-react"
import { toast } from "react-toastify"

// Components
import { Modal } from "@/app/_Components/Shared/Modal"
import { ActionDropdown } from "@/app/_Components/Shared/ActionDropdown"

// Server Actions
import { CreateAdmin } from "@/ServerActions/Admin/CreateAdmin"
import { GetAdmins } from "@/ServerActions/Admin/GetAdmins"
import { UpdateAdmin } from "@/ServerActions/Admin/UpdateAdmin"
import { DeleteAdmin } from "@/ServerActions/Admin/DeleteAdmin"

// Types
import { AdminResponse, CreateAdminData, UpdateAdminData } from "@/Types/AdminTypes"

const DEPARTMENTS = ["IT", "IS", "CS", "AI"] as const;

export default function AdminsLayout({ initialAdmins }: { initialAdmins: AdminResponse[] }) {
  const [data, setData] = useState<AdminResponse[]>(initialAdmins)
  const [searchQuery, setSearchQuery] = useState("")
  
  // Pagination State
  const [skip, setSkip] = useState(0)
  const [isLoadingMore, setIsLoadingMore] = useState(false)
  const [hasMore, setHasMore] = useState(initialAdmins.length === 100)
  const LIMIT = 100

  // Modals/UI State
  const [isDialogOpen, setIsDialogOpen] = useState(false)
  const [editingItem, setEditingItem] = useState<AdminResponse | null>(null)
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [itemToDelete, setItemToDelete] = useState<number | null>(null)
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false)

  const { register, handleSubmit, reset, formState: { errors } } = useForm<CreateAdminData>()

  const filteredData = data
    .filter(item =>
      item.full_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      item.department.toLowerCase().includes(searchQuery.toLowerCase()) ||
      item.admin_id.toString().includes(searchQuery)
    )
    .sort((a, b) => a.admin_id - b.admin_id)

  const handleLoadMore = async () => {
    setIsLoadingMore(true)
    try {
      const nextSkip = skip + LIMIT
      const newAdmins = await GetAdmins(nextSkip, LIMIT)
      if (newAdmins.length < LIMIT) setHasMore(false)
      setData((prev) => [...prev, ...newAdmins])
      setSkip(nextSkip)
    } catch (error) {
      toast.error("Failed to load more records")
    } finally {
      setIsLoadingMore(false)
    }
  }

  // --- UPDATED: Explicitly clearing email and password ---
  const handleOpenDialog = (item?: AdminResponse) => {
    if (item) {
      setEditingItem(item)
      reset({ 
        full_name: item.full_name, 
        title: item.title, 
        department: item.department,
        email: "",    // Clear on edit mode
        password: ""  // Clear on edit mode
      })
    } else {
      setEditingItem(null)
      reset({ 
        full_name: "", 
        title: "", 
        department: "CS",
        email: "",    // Clear on create mode
        password: ""  // Clear on create mode
      })
    }
    setIsDialogOpen(true)
  }

  const onSubmit = async (formData: CreateAdminData) => {
    setIsSubmitting(true);
    try {
      if (editingItem) {
        const updatePayload: UpdateAdminData = {
          full_name: formData.full_name,
          title: formData.title,
          department: formData.department
        }
        const result = await UpdateAdmin(editingItem.admin_id, updatePayload)
        setData(data.map(a => a.admin_id === editingItem.admin_id ? result : a))
        toast.success("Admin updated successfully")
      } else {
        const result = await CreateAdmin(formData)
        setData(prev => [result, ...prev])
        toast.success("Admin created successfully")
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
      await DeleteAdmin(itemToDelete)
      setData(data.filter(a => a.admin_id !== itemToDelete))
      toast.success("Admin removed")
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
          <div className="flex h-12 w-12 md:h-14 md:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-violet-500 shadow-xl shadow-blue-100">
            <ShieldCheck className="h-6 w-6 md:h-7 md:w-7 text-white" />
          </div>
          <div>
            <h1 className="text-xl md:text-3xl font-extrabold tracking-tight text-slate-900 leading-tight">Administrators</h1>
            <p className="text-xs md:text-base text-slate-500 font-medium">Manage personnel and access levels</p>
          </div>
        </div>
        <button
          onClick={() => handleOpenDialog()}
          className="flex w-full md:w-auto items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 px-6 py-3 text-sm font-bold text-white shadow-lg hover:shadow-indigo-200 hover:-translate-y-0.5 transition-all active:scale-95 cursor-pointer"
        >
          <Plus className="h-5 w-5" />
          Add Admin
        </button>
      </div>

      {/* Table Wrapper */}
      <div className="rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">
        {/* Search Bar */}
        <div className="p-4 md:p-6 border-b border-slate-50 bg-slate-50/30">
          <div className="relative w-full md:max-w-sm">
            <Search className="absolute left-3.5 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
            <input
              type="search"
              placeholder="Search by name, ID or department..."
              className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-blue-500/10 focus:border-blue-500 outline-none transition-all"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>
        </div>

        {/* Mobile Card List View */}
        <div className="md:hidden divide-y divide-slate-100">
          {filteredData.length === 0 ? (
            <div className="py-12 text-center text-slate-400 italic text-sm">No administrators found.</div>
          ) : (
            filteredData.map((admin) => (
              <div key={admin.admin_id} className="p-5 flex flex-col gap-4 hover:bg-slate-50/50 transition-colors">
                <div className="flex justify-between items-center">
                  <div className="flex items-center gap-2 text-[10px] font-mono font-bold text-indigo-600 bg-indigo-50 px-2.5 py-1 rounded-lg">
                    <Hash className="h-3 w-3" /> {admin.admin_id.toString().padStart(3, '0')}
                  </div>
                  {admin.admin_id !== 1 ? (
                    <ActionDropdown 
                      onEdit={() => handleOpenDialog(admin)} 
                      onDelete={() => { setItemToDelete(admin.admin_id); setIsDeleteModalOpen(true); }} 
                    />
                  ) : (
                    <ShieldAlert className="h-4 w-4 text-slate-300" />
                  )}
                </div>
                
                <div className="space-y-1">
                  <h3 className="text-sm font-bold text-slate-900">{admin.full_name}</h3>
                  <div className="flex flex-wrap gap-x-4 gap-y-2 pt-1">
                    <p className="text-xs text-slate-500 flex items-center gap-1.5">
                      <Briefcase className="h-3.5 w-3.5 text-slate-400" /> {admin.title}
                    </p>
                    <p className="text-xs text-slate-600 flex items-center gap-1.5 font-semibold uppercase tracking-wider">
                      <Building2 className="h-3.5 w-3.5 text-slate-400" /> {admin.department}
                    </p>
                  </div>
                </div>
              </div>
            ))
          )}
        </div>

        {/* Desktop Table View */}
        <div className="hidden md:block overflow-x-auto">
          <table className="w-full text-left">
            <thead>
              <tr className="bg-slate-50/50">
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">ID</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Full Name</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Role / Title</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Department</th>
                <th className="px-8 py-4 text-right text-xs font-bold uppercase tracking-widest text-slate-400">Actions</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-100">
              {filteredData.map((admin) => (
                <tr key={admin.admin_id} className="group hover:bg-indigo-50/20 transition-colors">
                  <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">
                    #{admin.admin_id.toString().padStart(3, '0')}
                  </td>
                  <td className="px-8 py-5">
                    <div className="text-sm font-bold text-slate-800">{admin.full_name}</div>
                    <div className="text-[10px] text-slate-400 uppercase tracking-tighter">UID: {admin.user_id}</div>
                  </td>
                  <td className="px-8 py-5 text-sm text-slate-600 font-medium">
                    <span className="flex items-center gap-2">
                      <Briefcase className="h-3.5 w-3.5 text-slate-400 shrink-0" />
                      {admin.title}
                    </span>
                  </td>
                  <td className="px-8 py-5">
                    <span className="inline-flex items-center gap-1.5 rounded-lg bg-indigo-50 px-3 py-1 text-[11px] font-bold text-indigo-600 border border-indigo-100 uppercase">
                      <Building2 className="h-3 w-3 shrink-0" />
                      {admin.department}
                    </span>
                  </td>
                  <td className="px-8 py-5 text-right">
                    {admin.admin_id !== 1 ? (
                      <ActionDropdown 
                        onEdit={() => handleOpenDialog(admin)} 
                        onDelete={() => { setItemToDelete(admin.admin_id); setIsDeleteModalOpen(true); }} 
                      />
                    ) : (
                      <div className="flex justify-end pr-2">
                        <span className="text-[10px] font-bold text-slate-300 uppercase tracking-widest flex items-center gap-1">
                          <ShieldAlert className="h-3 w-3" /> System
                        </span>
                      </div>
                    )}
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Footer */}
        <div className="p-5 md:p-6 bg-slate-50/30 border-t border-slate-100 flex flex-col md:flex-row items-center justify-between gap-4">
          <p className="text-sm text-slate-500 font-medium order-2 md:order-1">
            Total Records: <span className="text-slate-900 font-bold">{data.length}</span>
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

      {/* Main Modal Form */}
      <Modal
        open={isDialogOpen}
        onClose={() => !isSubmitting && setIsDialogOpen(false)}
        title={editingItem ? "Edit Administrator" : "New Administrator"}
        description={editingItem ? "Update personnel details." : "Register a new administrative account."}
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
                {...register("full_name", { required: "Name is required" })} 
                className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" 
                placeholder="Administrator Name" 
              />
            </div>
            {errors.full_name && <p className="text-red-500 text-[10px] mt-1 ml-1 font-semibold">{errors.full_name.message}</p>}
          </div>

          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div className="space-y-1.5">
              <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Title</label>
              <div className="relative">
                <Briefcase className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                <input {...register("title", { required: "Title is required" })} className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" placeholder="Dean" />
              </div>
              {errors.title && <p className="text-red-500 text-[10px] mt-1 ml-1 font-semibold">{errors.title.message}</p>}
            </div>
            <div className="space-y-1.5">
              <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Department</label>
              <div className="relative">
                <Building2 className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400 z-10" />
                <select 
                  {...register("department", { required: "Department is required" })}
                  className="w-full appearance-none rounded-xl border border-slate-200 pl-10 pr-10 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all bg-white cursor-pointer shadow-sm"
                >
                  {DEPARTMENTS.map((dept) => (
                    <option key={dept} value={dept}>{dept}</option>
                  ))}
                </select>
                <ChevronDown className="absolute right-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400 pointer-events-none" />
              </div>
            </div>
          </div>

          {/* Email and Password - Now explicitly cleared and autocomplete disabled */}
          {!editingItem && (
            <div className="pt-4 space-y-4 border-t border-slate-100">
              <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Email</label>
                <div className="relative">
                  <Mail className="absolute left-3.5 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                  <input 
                    {...register("email", { required: !editingItem })} 
                    type="email" 
                    autoComplete="off" // Disable browser autofill
                    className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3 text-sm outline-none focus:border-indigo-500 focus:ring-4 focus:ring-indigo-500/5 transition-all shadow-sm" 
                    placeholder="email@university.edu" 
                  />
                </div>
              </div>
              <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase tracking-wider ml-1">Password</label>
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
              {isSubmitting ? <Loader2 className="h-4 w-4 animate-spin" /> : (editingItem ? "Save Changes" : "Create Admin")}
            </button>
          </div>
        </form>
      </Modal>

      {/* Delete Modal */}
      <Modal open={isDeleteModalOpen} onClose={() => !isSubmitting && setIsDeleteModalOpen(false)} title="Confirm Deletion" description="Are you sure you want to remove this administrator? This action cannot be undone.">
        <div className="flex flex-col md:flex-row justify-end gap-3 mt-8">
          <button onClick={() => setIsDeleteModalOpen(false)} className="px-6 py-3 text-sm font-bold text-slate-500 order-2 md:order-1">Cancel</button>
          <button onClick={confirmDelete} disabled={isSubmitting} className="rounded-xl bg-red-600 px-8 py-3 text-sm font-bold text-white hover:bg-red-700 transition-colors order-1 md:order-2 shadow-lg shadow-red-100">
            {isSubmitting ? "Deleting..." : "Confirm Delete"}
          </button>
        </div>
      </Modal>
    </div>
  )
}