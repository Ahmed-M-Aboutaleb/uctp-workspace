"use client"

import { useState } from "react"
import { useForm } from "react-hook-form"
import { Plus, Search, Sparkles, Loader2, ArrowRight, Hash } from "lucide-react"
import { CreateFeature } from "@/ServerActions/Feature/CreateFeature"
import { toast } from "react-toastify"
import { DeleteFeature } from "@/ServerActions/Feature/DeleteFeature"
import { UpdateFeature } from "@/ServerActions/Feature/UpdateFeature"
import { GetFeatures } from "@/ServerActions/Feature/GetFeatures"
import { Modal } from "@/app/_Components/Shared/Modal"
import { ActionDropdown } from "@/app/_Components/Shared/ActionDropdown"
import { Feature, FeatureFormData } from "@/Types/FeaturesType"


export default function FeaturesLayout({ features }: { features: Feature[] }) {
  const [data, setData] = useState<Feature[]>(features)
  const [searchQuery, setSearchQuery] = useState("")
  
  // Pagination State
  const [skip, setSkip] = useState(0)
  const [isLoadingMore, setIsLoadingMore] = useState(false)
  const [hasMore, setHasMore] = useState(features.length === 100)
  const LIMIT = 100

  const [isDialogOpen, setIsDialogOpen] = useState(false)
  const [editingItem, setEditingItem] = useState<Feature | null>(null)
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [itemToDelete, setItemToDelete] = useState<number | null>(null)
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false)

  const { register, handleSubmit, reset, formState: { errors } } = useForm<FeatureFormData>()

  const filteredData = data
    .filter(item =>
      item.feature_name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      item.feature_id.toString().includes(searchQuery)
    )
    .sort((a, b) => a.feature_id - b.feature_id)

  const handleLoadMore = async () => {
    setIsLoadingMore(true)
    try {
      const nextSkip = skip + LIMIT
      const newFeatures = await GetFeatures(nextSkip, LIMIT)
      
      if (newFeatures.length < LIMIT) {
        setHasMore(false)
      }

      setData((prev) => [...prev, ...newFeatures])
      setSkip(nextSkip)
    } catch (error) {
      toast.error("Failed to load more features")
    } finally {
      setIsLoadingMore(false)
    }
  }

  const handleOpenDialog = (item?: Feature) => {
    if (item) {
      setEditingItem(item)
      reset({ feature_name: item.feature_name })
    } else {
      setEditingItem(null)
      reset({ feature_name: "" })
    }
    setIsDialogOpen(true)
  }

  const onSubmit = async (formData: FeatureFormData) => {
    setIsSubmitting(true)
    try {
      if (editingItem) {
        const result = await UpdateFeature(editingItem.feature_id, formData.feature_name)
        if (result && result.error) {
          toast.error(result.error)
        } else {
          setData(data.map(f => 
            f.feature_id === editingItem.feature_id ? { ...f, feature_name: formData.feature_name } : f
          ))
          toast.success("Feature updated successfully.")
          setIsDialogOpen(false)
        }
      } else {
        const result = await CreateFeature(formData.feature_name)
        if (result && 'error' in result) {
          toast.error(result.error)
        } else {
          const newFeature = result as unknown as Feature
          setData(prev => [...prev, newFeature])
          toast.success("Feature created successfully.")
          setIsDialogOpen(false)
          reset()
        }
      }
    } catch (error) {
      toast.error("An error occurred while saving.")
    } finally {
      setIsSubmitting(false)
    }
  }

  const confirmDelete = async () => {
    if (itemToDelete) {
      setIsSubmitting(true)
      try {
        const success = await DeleteFeature(itemToDelete)
        if (success) {
          setData(data.filter(f => f.feature_id !== itemToDelete))
          toast.success("Feature deleted successfully.")
          setIsDeleteModalOpen(false)
          setItemToDelete(null)
        }
      } catch (error: any) {
        toast.error(error.message || "Failed to delete.")
      } finally {
        setIsSubmitting(false)
      }
    }
  }

  return (
    <div className="max-w-6xl mx-auto p-4 sm:p-8 space-y-6 sm:space-y-8 animate-in fade-in duration-500">
      
      {/* Header Section */}
      <div className="flex flex-col gap-6 md:flex-row md:items-center md:justify-between border-b border-slate-100 pb-8">
        <div className="flex items-center gap-4">
          <div className="flex h-12 w-12 sm:h-14 sm:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-violet-500 shadow-xl shadow-blue-200">
            <Sparkles className="h-6 w-6 sm:h-7 sm:w-7 text-white" />
          </div>
          <div>
            <h1 className="text-2xl sm:text-3xl font-extrabold tracking-tight text-slate-900">Features</h1>
            <p className="text-sm sm:text-base text-slate-500 font-medium">Manage platform features and capabilities</p>
          </div>
        </div>
        <button
          onClick={() => handleOpenDialog()}
          className="flex items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 px-6 py-3.5 text-sm font-bold text-white shadow-lg hover:shadow-primary/20 hover:-translate-y-0.5 transition-all active:scale-95 cursor-pointer w-full md:w-auto"
        >
          <Plus className="h-5 w-5" />
          Add Feature
        </button>
      </div>

      {/* Main Table/List Card */}
      <div className="rounded-2xl sm:rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">
        <div className="p-4 sm:p-6 border-b border-slate-50 bg-slate-50/30">
          <div className="relative w-full sm:max-w-sm">
            <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
            <input
              type="search"
              placeholder="Filter by name or ID..."
              className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-blue-500/10 focus:border-blue-500 outline-none transition-all"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>
        </div>

        {/* Mobile View: Cards */}
        <div className="block md:hidden divide-y divide-slate-100">
          {filteredData.length === 0 ? (
            <div className="py-12 text-center text-slate-400 italic">No features found.</div>
          ) : (
            filteredData.map((item) => (
              <div key={item.feature_id} className="p-4 flex items-center justify-between hover:bg-slate-50 transition-colors">
                <div className="flex flex-col gap-1">
                   <div className="flex items-center gap-2 text-[10px] font-mono font-bold text-indigo-500 bg-indigo-50 px-2 py-0.5 rounded-full w-fit">
                    <Hash className="h-2.5 w-2.5" />
                    {item.feature_id.toString().padStart(3, '0')}
                   </div>
                   <span className="text-sm font-bold text-slate-700">{item.feature_name}</span>
                </div>
                <ActionDropdown 
                  onEdit={() => handleOpenDialog(item)} 
                  onDelete={() => { setItemToDelete(item.feature_id); setIsDeleteModalOpen(true); }} 
                />
              </div>
            ))
          )}
        </div>

        {/* Desktop View: Table */}
        <div className="hidden md:block overflow-x-auto">
          <table className="w-full text-left">
            <thead>
              <tr className="bg-slate-50/50">
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">ID</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Feature Name</th>
                <th className="px-8 py-4 text-right text-xs font-bold uppercase tracking-widest text-slate-400">Actions</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-100">
              {filteredData.length === 0 ? (
                <tr>
                  <td colSpan={3} className="py-20 text-center text-slate-400 italic">No features found.</td>
                </tr>
              ) : (
                filteredData.map((item) => (
                  <tr key={item.feature_id} className="group hover:bg-indigo-50/30 transition-colors">
                    <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">
                      #{item.feature_id.toString().padStart(3, '0')}
                    </td>
                    <td className="px-8 py-5 text-sm font-bold text-slate-700">
                      {item.feature_name}
                    </td>
                    <td className="px-8 py-5 text-right">
                      <ActionDropdown 
                        onEdit={() => handleOpenDialog(item)} 
                        onDelete={() => { setItemToDelete(item.feature_id); setIsDeleteModalOpen(true); }} 
                      />
                    </td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
        </div>

        {/* Load More Pagination Footer */}
        <div className="p-4 sm:p-6 bg-slate-50/30 border-t border-slate-100 flex flex-col sm:flex-row items-center justify-between gap-4">
          <p className="text-sm text-slate-500 font-medium order-2 sm:order-1">
            Showing <span className="text-slate-900 font-bold">{data.length}</span> features
          </p>
          {hasMore && (
            <button
              onClick={handleLoadMore}
              disabled={isLoadingMore}
              className="flex items-center justify-center gap-2 rounded-xl border border-slate-200 bg-white px-5 py-2.5 text-sm font-bold text-slate-700 shadow-sm hover:bg-slate-50 transition-all active:scale-95 disabled:opacity-50 cursor-pointer w-full sm:w-auto order-1 sm:order-2"
            >
              {isLoadingMore ? (
                <Loader2 className="h-4 w-4 animate-spin" />
              ) : (
                <>
                  Load Next 100
                  <ArrowRight className="h-4 w-4" />
                </>
              )}
            </button>
          )}
        </div>
      </div>

      {/* Form Modal */}
      <Modal
        open={isDialogOpen}
        onClose={() => !isSubmitting && setIsDialogOpen(false)}
        title={editingItem ? "Edit Feature" : "New Feature"}
        description="Enter the unique name for this system capability."
      >
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-5">
          <div className="space-y-2">
            <label className="text-sm font-bold text-slate-700">Feature Name</label>
            <input
              {...register("feature_name", { required: "Feature name is required" })}
              autoFocus
              disabled={isSubmitting}
              type="text"
              placeholder="e.g. Wireless Casting"
              className={`w-full rounded-xl border px-4 py-3 text-sm outline-none transition-all ${
                errors.feature_name ? "border-red-500 focus:ring-red-500/10" : "border-slate-200 focus:ring-indigo-500/10 focus:border-indigo-500"
              }`}
            />
            {errors.feature_name && (
              <p className="text-xs text-red-500 font-medium">{errors.feature_name.message}</p>
            )}
          </div>
          <div className="flex flex-col-reverse sm:flex-row justify-end gap-3 pt-2">
            <button
              type="button"
              disabled={isSubmitting}
              onClick={() => setIsDialogOpen(false)}
              className="px-5 py-3 text-sm font-bold text-slate-500 hover:bg-slate-100 rounded-xl transition-colors disabled:opacity-50 cursor-pointer w-full sm:w-auto"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={isSubmitting}
              className="flex items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 px-6 py-3 text-sm font-bold text-white hover:opacity-90 shadow-md active:scale-95 transition-all disabled:opacity-50 cursor-pointer w-full sm:w-auto"
            >
              {isSubmitting && <Loader2 className="h-4 w-4 animate-spin" />}
              {editingItem ? "Update Changes" : "Save Feature"}
            </button>
          </div>
        </form>
      </Modal>

      {/* Delete Confirmation Modal */}
      <Modal open={isDeleteModalOpen} onClose={() => setIsDeleteModalOpen(false)} title="Confirm Deletion" description="Are you sure you want to delete this feature? This action cannot be undone.">
        <div className="flex flex-col-reverse sm:flex-row justify-end gap-3 mt-6">
          <button 
            onClick={() => setIsDeleteModalOpen(false)} 
            className="px-5 py-3 text-sm font-bold text-slate-500 cursor-pointer w-full sm:w-auto rounded-xl hover:bg-slate-50"
          >
            Cancel
          </button>
          <button 
            onClick={confirmDelete} 
            disabled={isSubmitting}
            className="rounded-xl bg-red-600 px-6 py-3 text-sm font-bold text-white hover:bg-red-700 disabled:opacity-50 cursor-pointer w-full sm:w-auto"
          >
            {isSubmitting ? "Deleting..." : "Confirm Delete"}
          </button>
        </div>
      </Modal>
    </div>
  )
}