﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.34011
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using System.Xml.Serialization;

// 
// This source code was auto-generated by xsd, Version=4.0.30319.1.
// 


/// <remarks/>
[System.CodeDom.Compiler.GeneratedCodeAttribute("xsd", "4.0.30319.1")]
[System.SerializableAttribute()]
[System.Diagnostics.DebuggerStepThroughAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType=true)]
[System.Xml.Serialization.XmlRootAttribute(Namespace="", IsNullable=false)]
public partial class Files {
    
    private FilesFile[] fileField;
    
    /// <remarks/>
    [System.Xml.Serialization.XmlElementAttribute("File")]
    public FilesFile[] File {
        get {
            return this.fileField;
        }
        set {
            this.fileField = value;
        }
    }
}

/// <remarks/>
[System.CodeDom.Compiler.GeneratedCodeAttribute("xsd", "4.0.30319.1")]
[System.SerializableAttribute()]
[System.Diagnostics.DebuggerStepThroughAttribute()]
[System.ComponentModel.DesignerCategoryAttribute("code")]
[System.Xml.Serialization.XmlTypeAttribute(AnonymousType=true)]
public partial class FilesFile {
    
    private string filenameField;
    
    private long sizeBytesField;
    
    private long createdDateTicksField;
    
    private double maxAlarmField;
    
    private int durationSecondsField;
    
    private bool isTimelapseField;
    
    private bool isMergeFileField;
    
    private bool isMergeFileFieldSpecified;
    
    private string alertDataField;
    
    private double triggerLevelField;
    
    private double triggerLevelMaxField;
    
    private bool triggerLevelMaxFieldSpecified;
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public string Filename {
        get {
            return this.filenameField;
        }
        set {
            this.filenameField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public long SizeBytes {
        get {
            return this.sizeBytesField;
        }
        set {
            this.sizeBytesField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public long CreatedDateTicks {
        get {
            return this.createdDateTicksField;
        }
        set {
            this.createdDateTicksField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public double MaxAlarm {
        get {
            return this.maxAlarmField;
        }
        set {
            this.maxAlarmField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public int DurationSeconds {
        get {
            return this.durationSecondsField;
        }
        set {
            this.durationSecondsField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public bool IsTimelapse {
        get {
            return this.isTimelapseField;
        }
        set {
            this.isTimelapseField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public bool IsMergeFile {
        get {
            return this.isMergeFileField;
        }
        set {
            this.isMergeFileField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlIgnoreAttribute()]
    public bool IsMergeFileSpecified {
        get {
            return this.isMergeFileFieldSpecified;
        }
        set {
            this.isMergeFileFieldSpecified = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public string AlertData {
        get {
            return this.alertDataField;
        }
        set {
            this.alertDataField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public double TriggerLevel {
        get {
            return this.triggerLevelField;
        }
        set {
            this.triggerLevelField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlAttributeAttribute()]
    public double TriggerLevelMax {
        get {
            return this.triggerLevelMaxField;
        }
        set {
            this.triggerLevelMaxField = value;
        }
    }
    
    /// <remarks/>
    [System.Xml.Serialization.XmlIgnoreAttribute()]
    public bool TriggerLevelMaxSpecified {
        get {
            return this.triggerLevelMaxFieldSpecified;
        }
        set {
            this.triggerLevelMaxFieldSpecified = value;
        }
    }
}
