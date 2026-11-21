USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtSkill]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtSkill](
	[id] [int] NULL,
	[_NameID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StaticName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IconImageIndex] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_BuffIconImageIndex] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DeBuffIconImageIndex] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedJob] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedWeaponType1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedWeaponType2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MaxLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DurationType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_TargetType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Dissolvable] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Element] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass1ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass2ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass3ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass4ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass5ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectAmassCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillDuplicate] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectDuplicate] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UsableChecker1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UsableChecker2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UsableChecker3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UsableChecker4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UsableChecker5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Processor1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Processor2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Processor3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Processor4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Processor5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Lock] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UnlockSkillBookItemID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_UnlockPrice] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DuplicatedSkillType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_BaseSkillID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillTip] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass6ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EffectClass7ApplyType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Group] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GlobalCoolTimePvE] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GlobalCoolTimePvP] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GlobalSkillGroup] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_IconPriority] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkillGroup] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIString_NameID] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtSkill_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtSkill_id] ON [dbo].[ExtSkill]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
