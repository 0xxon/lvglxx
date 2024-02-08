// Please see "LICENSE" file for copyright and license information.

#pragma once

#include <lvgl.h>

#include <string>
#include <vector>
#include <functional>

namespace lvglxx {

// Helper classes for callbacks
template<class WrapperType>
using ui_event_callback = std::function<void (WrapperType&, lv_event_t*)>;

class UiEventDispatchBase {
public:
    virtual void Dispatch(lv_event_t*);
    static void EventCallback(lv_event_t* e) {
        UiEventDispatchBase* dp = static_cast<UiEventDispatchBase*>(lv_event_get_user_data(e));
        dp->Dispatch(e);
    }
};

template <class WrapperType>
class UiEventDispatch : public UiEventDispatchBase {
public:
    UiEventDispatch(WrapperType& w, ui_event_callback<WrapperType> ev) : wrapper_(w), cb_(ev) {}
    void Dispatch(lv_event_t* e) override { cb_(wrapper_, e); }
private:
    WrapperType& wrapper_;
    ui_event_callback<WrapperType> cb_;
};

template <typename Child>
class BaseWrapper {
public:
    BaseWrapper() = default;

    Child& SetFlexFlow(lv_flex_flow_t flow = LV_FLEX_FLOW_ROW_WRAP) { lv_obj_set_flex_flow(obj_, flow); return static_cast<Child&>(*this); }
    Child& SetFlexAlign(lv_flex_align_t main_place = LV_FLEX_ALIGN_START, lv_flex_align_t cross_place = LV_FLEX_ALIGN_CENTER, lv_flex_align_t track_cross_place = LV_FLEX_ALIGN_START) { lv_obj_set_flex_align(obj_, main_place, cross_place, track_cross_place); return static_cast<Child&>(*this);  }
    Child& SetAlign(lv_align_t align) { lv_obj_set_align(obj_, align); return static_cast<Child&>(*this); }
    Child& SetFlexGrow(int32_t grow = 1) { lv_obj_set_flex_grow(obj_, grow); return static_cast<Child&>(*this); }
    Child& SetFlexInNewTrack() { lv_obj_add_flag(obj_, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK); return static_cast<Child&>(*this); }
    Child& FullWidth() { lv_obj_set_width(obj_, lv_pct(100)); return static_cast<Child&>(*this); }
    Child& SetWidth(int32_t width = LV_SIZE_CONTENT) { lv_obj_set_width(obj_, width); return static_cast<Child&>(*this); }
    Child& SetHeight(int32_t height = LV_SIZE_CONTENT) { lv_obj_set_height(obj_, height); return static_cast<Child&>(*this); }
    Child& SetX(int32_t x) { lv_obj_set_x(obj_, x); return static_cast<Child&>(*this); }
    Child& SetY(int32_t y) { lv_obj_set_y(obj_, y); return static_cast<Child&>(*this); }
    Child& AddState(lv_state_t state) { lv_obj_add_state(obj_, state); return static_cast<Child&>(*this); }
    Child& RemoveState(lv_state_t state) { lv_obj_remove_state(obj_, state); return static_cast<Child&>(*this); }
    Child& RemoveStyleAll() { lv_obj_remove_style_all(obj_); return static_cast<Child&>(*this); }
    Child& SetTextColor(lv_color_t color) { lv_obj_set_style_text_color(obj_, color, LV_PART_MAIN); return static_cast<Child&>(*this); }
    Child& PadRow(int32_t pad) { lv_obj_set_style_pad_row(obj_, pad, LV_PART_MAIN); return static_cast<Child&>(*this); }
    Child& PadColumn(int32_t pad) { lv_obj_set_style_pad_column(obj_, pad, LV_PART_MAIN); return static_cast<Child&>(*this); }
    Child& SetObject(lv_obj_t* object) { obj_ = object; return static_cast<Child&>(*this); }
    Child& AddCallback(ui_event_callback<Child> cb, lv_event_code_t filter) {
        auto callback = new UiEventDispatch<Child>(static_cast<Child&>(*this), cb);
        lv_obj_add_event_cb(obj_, UiEventDispatchBase::EventCallback, filter, callback);
        return static_cast<Child&>(*this);
    }
    Child& AddCallback(std::function<void ()> cb, lv_event_code_t filter) {
        AddCallback([=](Child&, lv_event_t*){cb();}, filter);
        return static_cast<Child&>(*this);
    }
    //operator lv_obj_t*() { return obj_; }
    lv_obj_t* GetObj() { return obj_; }

protected:
    lv_obj_t* obj_ = nullptr;
};

class Object : public BaseWrapper<Object>{
public:
    explicit Object(lv_obj_t* parent) { obj_ = lv_obj_create(parent); }
};

template <typename Child>
class LabelBase : public BaseWrapper<Child> {
public:
    LabelBase() = default;
    explicit LabelBase(lv_obj_t* parent) {
        this->obj_ = lv_label_create(parent);
    }

    LabelBase(lv_obj_t* parent, std::string_view text) {
        this->obj_ = lv_label_create(parent);
        lv_label_set_text(this->obj_, text.data());
    }

    Child& SetText(std::string_view text) { lv_label_set_text(this->obj_, text.data()); return static_cast<Child&>(*this); }
};

class Label : public LabelBase<Label> {
public:
    Label() = default;
    explicit Label(lv_obj_t* parent) : LabelBase(parent) {}
    Label(lv_obj_t* parent, std::string_view text) : LabelBase(parent, text) {}
};

class Button : public BaseWrapper<Button> {
public:
    Button() = default;
    explicit Button(lv_obj_t* parent) {
        obj_ = lv_button_create(parent);
        thelabel = Label(obj_);
        thelabel.SetWidth().SetHeight().SetText(""); // .SetAlign(LV_ALIGN_CENTER);
    }
    Button(lv_obj_t* parent, std::string_view text) : Button(parent) {
        SetText(text);
    }

    Label& GetLabel() { return thelabel; }
    Button& SetText(std::string_view text) { thelabel.SetText(text); return *this; }

private:
    Label thelabel;
};

class Bar : public BaseWrapper<Bar> {
public:
    Bar() = default;
    explicit Bar(lv_obj_t* parent) {
        obj_ = lv_bar_create(parent);
        thelabel = Label(obj_);
        thelabel.SetWidth().SetHeight().SetText("").SetAlign(LV_ALIGN_CENTER);
    }

    Label& GetLabel() { return thelabel; }

    Bar& SetValue(int32_t val) { lv_bar_set_value(obj_, val, LV_ANIM_OFF); return *this; }
    Bar& SetLabel(std::string_view text) { thelabel.SetText(text); return *this; }

private:
    Label thelabel;
};

class Dropdown : public BaseWrapper<Dropdown> {
public:
    Dropdown() = default;
    explicit Dropdown(lv_obj_t* parent) {
        obj_ = lv_dropdown_create(parent);
    }
    Dropdown(lv_obj_t* parent, std::vector<std::string> options) : Dropdown(parent) {
        SetOptions(options);
    }

    Dropdown& SetOptions(std::vector<std::string> options) {
        std::string out;
        for ( auto& el : options )
            out += el + "\n";
        lv_dropdown_set_options(obj_, out.c_str());
        return *this; 
    }

    Dropdown& SetSelected(uint32_t s) { lv_dropdown_set_selected(obj_, s); return *this; }
    uint32_t GetSelected() { return lv_dropdown_get_selected(obj_); }
};

} // namespace lvglxx