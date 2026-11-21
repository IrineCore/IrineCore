USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtWeapon]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtWeapon](
	[id] [int] NULL,
	[_EquipType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkinName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ActName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AniName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DecalName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SimName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Length] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Durability] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_DurabilityRepairCoin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Normal_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Critical_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Stun_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Normal_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Critical_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Stun_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Normal_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Critical_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Stun_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Defense_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Finish_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Normal_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Normal_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Normal_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Critical_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Stun_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Defense_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Finish_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Sound_Range] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Sound_Rolloff] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EmptyDurabilitySound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SetItemID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SetEffect_ActName] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Grade_SetItem] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Normal_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Critical_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material1_Stun_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Normal_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Critical_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material2_Stun_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Normal_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Critical_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material3_Stun_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Defense_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Finish_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EquipTypeSub1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkinNameSub1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ActNameSub1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AniNameSub1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SimNameSub1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_EquipTypeSub2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SkinNameSub2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ActNameSub2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AniNameSub2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SimNameSub2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SetItemTextID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Normal_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Critical_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Stun_Particle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Normal_Sound] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Normal_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Critical_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Material4_Stun_Action] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_OneType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SetOverlap] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtWeapon_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtWeapon_id] ON [dbo].[ExtWeapon]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
