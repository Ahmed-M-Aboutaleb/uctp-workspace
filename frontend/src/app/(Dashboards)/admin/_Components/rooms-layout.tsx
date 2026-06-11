"use client"

import { CreateRoom } from "@/ServerActions/Room/CreateRoom"
import { DeleteRoom } from "@/ServerActions/Room/DeleteRoom"
import { UpdateRoom } from "@/ServerActions/Room/UpdateRoom"
import { GetRooms } from "@/ServerActions/Room/GetRooms"
import { Feature, Room, RoomFormData } from "@/Types/RoomsType"
import { DoorOpen, Loader2, Pencil, Plus, Search, Trash2, Users, X, ArrowRight, Settings2 } from "lucide-react"
import { useState } from "react"
import { useForm } from "react-hook-form"
import { toast } from 'react-toastify';
import { ActionDropdown } from "@/app/_Components/Shared/ActionDropdown"
import { Modal } from "@/app/_Components/Shared/Modal"

export default function RoomsLayout({ rooms, availableFeatures }: { rooms: Room[], availableFeatures: Feature[] }) {
  const [data, setData] = useState<Room[]>(rooms)
  const [searchQuery, setSearchQuery] = useState("")
  
  // Pagination State
  const [skip, setSkip] = useState(0)
  const [isLoadingMore, setIsLoadingMore] = useState(false)
  const [hasMore, setHasMore] = useState(rooms.length === 100)
  const LIMIT = 100

  const [isDialogOpen, setIsDialogOpen] = useState(false)
  const [editingItem, setEditingItem] = useState<Room | null>(null)
  
  const [isDeleteModalOpen, setIsDeleteModalOpen] = useState(false)
  const [itemToDelete, setItemToDelete] = useState<number | null>(null)
  
  const [isSubmitting, setIsSubmitting] = useState(false)

  const { register, handleSubmit, reset, formState: { errors } } = useForm<RoomFormData>()

  const filteredData = data
    .filter(item => 
      item.room_id.toString().includes(searchQuery) ||
      item.features.some(f => f.feature_name.toLowerCase().includes(searchQuery.toLowerCase()))
    )
    .sort((a, b) => a.room_id - b.room_id)

  const handleLoadMore = async () => {
    setIsLoadingMore(true)
    try {
      const nextSkip = skip + LIMIT
      const newRooms = await GetRooms(nextSkip, LIMIT)
      if (newRooms.length < LIMIT) setHasMore(false)
      setData((prev) => [...prev, ...newRooms])
      setSkip(nextSkip)
    } catch (error) {
      toast.error("Failed to load more rooms")
    } finally {
      setIsLoadingMore(false)
    }
  }

  const handleOpenDialog = (room?: Room) => {
    if (room) {
      setEditingItem(room)
      reset({ 
        capacity: room.capacity, 
        feature_ids: room.features.map(f => f.feature_id) 
      })
    } else {
      setEditingItem(null)
      reset({ capacity: 0, feature_ids: [] })
    }
    setIsDialogOpen(true)
  }

  const onSubmit = async (formData: RoomFormData) => {
    setIsSubmitting(true)
    try {
      const numericFeatureIds = formData.feature_ids.map(Number);
      if (editingItem) {
        const updatedRoomFromServer = await UpdateRoom(editingItem.room_id, formData.capacity, numericFeatureIds);
        setData((prev) => prev.map((r) => r.room_id === editingItem.room_id ? updatedRoomFromServer : r));
        toast.success("Room updated successfully");
      } else {
        const newRoomFromServer = await CreateRoom(formData);
        setData((prev) => [...prev, newRoomFromServer]);
        toast.success("Room created successfully");
      }
      setIsDialogOpen(false)
    } finally {
      setIsSubmitting(false)
    }
  }

  const handleDeleteClick = (id: number) => {
    setItemToDelete(id)
    setIsDeleteModalOpen(true)
  }

  const confirmDelete = async () => {
    if (itemToDelete) {
      setIsSubmitting(true);
      try {
        const success = await DeleteRoom(itemToDelete);
        if (success) {
          setData(data.filter(r => r.room_id !== itemToDelete));
          toast.success("Room deleted successfully.");
          setIsDeleteModalOpen(false);
          setItemToDelete(null);
        }
      } catch (error: any) {
        toast.error(error.message || "Failed to delete room.");
      } finally {
        setIsSubmitting(false);
      }
    }
  }

  return (
    <div className="max-w-6xl mx-auto p-4 md:p-8 space-y-6 md:space-y-8 animate-in fade-in duration-500">
      
      {/* Header - Stacked on mobile, row on desktop */}
      <div className="flex flex-col gap-6 md:flex-row md:items-center md:justify-between border-b border-slate-100 pb-6 md:pb-8">
        <div className="flex items-center gap-4">
          <div className="flex h-12 w-12 md:h-14 md:w-14 shrink-0 items-center justify-center rounded-2xl bg-gradient-to-r from-indigo-600 to-violet-500 shadow-xl shadow-indigo-100">
            <DoorOpen className="h-6 w-6 md:h-7 md:w-7 text-white" />
          </div>
          <div>
            <h1 className="text-2xl md:text-3xl font-extrabold tracking-tight text-slate-900">Rooms</h1>
            <p className="text-sm md:text-base text-slate-500 font-medium leading-tight">Manage facility rooms and equipment</p>
          </div>
        </div>
        <button
          onClick={() => handleOpenDialog()}
          className="w-full md:w-auto flex items-center justify-center gap-2 rounded-xl bg-gradient-to-r from-indigo-600 to-violet-500 cursor-pointer px-6 py-3.5 text-sm font-bold text-white shadow-lg hover:shadow-indigo-200 transition-all active:scale-95"
        >
          <Plus className="h-5 w-5" />
          Add Room
        </button>
      </div>

      {/* Search & Stats Section */}
      <div className="rounded-2xl md:rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">
        <div className="p-4 md:p-6 border-b border-slate-50 bg-slate-50/30">
          <div className="relative w-full md:max-w-sm">
            <Search className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
            <input
              type="search"
              placeholder="Filter by ID or features..."
              className="h-11 w-full rounded-xl border border-slate-200 bg-white pl-10 pr-4 text-sm focus:ring-4 focus:ring-indigo-500/10 focus:border-indigo-500 outline-none transition-all"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </div>
        </div>

        {/* Responsive List: Mobile Cards / Desktop Table */}
        <div className="block md:hidden">
          {filteredData.map((room) => (
            <div key={room.room_id} className="p-4 border-b border-slate-100 last:border-0">
              <div className="flex justify-between items-start mb-3">
                <span className="text-xs font-mono text-indigo-600 font-bold bg-indigo-50 px-2 py-1 rounded">
                  #{room.room_id.toString().padStart(3, '0')}
                </span>
                <ActionDropdown 
                  onEdit={() => handleOpenDialog(room)} 
                  onDelete={() => handleDeleteClick(room.room_id)} 
                />
              </div>
              <div className="flex items-center gap-2 text-sm font-bold text-slate-700 mb-3">
                <Users className="h-4 w-4 text-slate-400" />
                Capacity: {room.capacity}
              </div>
              <div className="flex flex-wrap gap-1.5">
                {room.features.map(f => (
                  <span key={f.feature_id} className="inline-flex items-center rounded-md bg-slate-100 px-2 py-1 text-[10px] font-bold uppercase text-slate-500">
                    {f.feature_name}
                  </span>
                ))}
              </div>
            </div>
          ))}
        </div>

        <div className="hidden md:block overflow-x-auto">
          <table className="w-full text-left">
            <thead>
              <tr className="bg-slate-50/50">
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Room ID</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Capacity</th>
                <th className="px-8 py-4 text-xs font-bold uppercase tracking-widest text-slate-400">Features</th>
                <th className="px-8 py-4 text-right text-xs font-bold uppercase tracking-widest text-slate-400">Actions</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-100">
              {filteredData.map((room) => (
                <tr key={room.room_id} className="group hover:bg-indigo-50/30 transition-colors">
                  <td className="px-8 py-5 text-sm font-mono text-indigo-600 font-semibold">#{room.room_id.toString().padStart(3, '0')}</td>
                  <td className="px-8 py-5">
                    <div className="flex items-center gap-2 text-sm font-bold text-slate-700">
                      <Users className="h-4 w-4 text-slate-400" />
                      {room.capacity}
                    </div>
                  </td>
                  <td className="px-8 py-5">
                    <div className="flex flex-wrap gap-1.5">
                      {room.features.map(f => (
                        <span key={f.feature_id} className="inline-flex items-center rounded-md bg-slate-100 px-2 py-1 text-xs font-medium text-slate-600">
                          {f.feature_name}
                        </span>
                      ))}
                    </div>
                  </td>
                  <td className="px-8 py-5 text-right">
                    <ActionDropdown 
                      onEdit={() => handleOpenDialog(room)} 
                      onDelete={() => handleDeleteClick(room.room_id)} 
                    />
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {/* Pagination Footer */}
        <div className="p-4 md:p-6 bg-slate-50/30 border-t border-slate-100 flex flex-col md:flex-row items-center justify-between gap-4">
          <p className="text-sm text-slate-500 font-medium">
            Showing <span className="text-slate-900 font-bold">{filteredData.length}</span> rooms
          </p>
          {hasMore && (
            <button
              onClick={handleLoadMore}
              disabled={isLoadingMore}
              className="w-full md:w-auto flex items-center justify-center gap-2 rounded-xl border border-slate-200 bg-white px-5 py-2.5 text-sm font-bold text-slate-700 shadow-sm hover:bg-slate-50 transition-all active:scale-95 disabled:opacity-50 cursor-pointer"
            >
              {isLoadingMore ? <Loader2 className="h-4 w-4 animate-spin" /> : <>Load More <ArrowRight className="h-4 w-4" /></>}
            </button>
          )}
        </div>
      </div>

      {/* Main Form Modal */}
      <Modal
        open={isDialogOpen}
        onClose={() => !isSubmitting && setIsDialogOpen(false)}
        title={editingItem ? "Edit Room" : "New Room"}
        description="Configure the room capacity and available equipment."
      >
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-6">
          <div className="space-y-2">
            <label className="text-sm font-bold text-slate-700">Capacity</label>
            <div className="relative">
              <Users className="absolute left-3 top-1/2 h-4 w-4 -translate-y-1/2 text-slate-400" />
              <input
                {...register("capacity", { required: "Capacity is required", min: 1 })}
                type="number"
                placeholder="e.g. 50"
                className="w-full rounded-xl border border-slate-200 pl-10 pr-4 py-3.5 text-sm focus:ring-4 focus:ring-indigo-500/10 outline-none transition-all"
              />
            </div>
          </div>

          <div className="space-y-2">
            <label className="text-sm font-bold text-slate-700">Available Features</label>
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-2 md:gap-3 max-h-[40vh] overflow-y-auto p-1 custom-scrollbar">
              {availableFeatures.map(feature => (
                <label key={feature.feature_id} className="flex items-center gap-3 p-3 rounded-xl border border-slate-100 hover:bg-indigo-50/50 cursor-pointer transition-colors group">
                  <input
                    type="checkbox"
                    value={feature.feature_id}
                    {...register("feature_ids")}
                    className="h-5 w-5 rounded-md border-slate-300 text-indigo-600 focus:ring-indigo-500"
                  />
                  <span className="text-sm font-medium text-slate-600 group-hover:text-indigo-700 transition-colors">{feature.feature_name}</span>
                </label>
              ))}
            </div>
          </div>

          <div className="flex flex-col-reverse sm:flex-row justify-end gap-3 pt-2">
            <button 
              type="button" 
              onClick={() => setIsDialogOpen(false)} 
              className="w-full sm:w-auto px-5 py-3 text-sm font-bold text-slate-500 hover:bg-slate-100 rounded-xl transition-colors"
            >
              Cancel
            </button>
            <button 
              type="submit" 
              disabled={isSubmitting}
              className="w-full sm:w-auto flex items-center justify-center gap-2 rounded-xl bg-indigo-600 px-8 py-3 text-sm font-bold text-white hover:bg-indigo-700 transition-all active:scale-95 disabled:opacity-70 shadow-lg shadow-indigo-100"
            >
              {isSubmitting && <Loader2 className="h-4 w-4 animate-spin" />}
              {editingItem ? "Update Room" : "Save Room"}
            </button>
          </div>
        </form>
      </Modal>

      {/* Delete Confirmation Modal */}
      <Modal open={isDeleteModalOpen} onClose={() => setIsDeleteModalOpen(false)} title="Confirm Deletion" description="Are you sure you want to delete this room? This action cannot be undone.">
        <div className="flex flex-col-reverse sm:flex-row justify-end gap-3 mt-8">
          <button onClick={() => setIsDeleteModalOpen(false)} className="w-full sm:w-auto px-5 py-3 text-sm font-bold text-slate-500 hover:bg-slate-50 rounded-xl">Cancel</button>
          <button 
            onClick={confirmDelete} 
            disabled={isSubmitting}
            className="w-full sm:w-auto rounded-xl bg-red-600 px-8 py-3 text-sm font-bold text-white hover:bg-red-700 disabled:opacity-50 transition-colors shadow-lg shadow-red-100"
          >
            {isSubmitting ? "Deleting..." : "Confirm Delete"}
          </button>
        </div>
      </Modal>
    </div>
  )
}