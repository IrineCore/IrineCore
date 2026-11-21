USE [dnworld]
GO
/****** Object:  Table [dbo].[PetEquipments]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetEquipments](
	[PetItemSerial] [bigint] NOT NULL,
	[PetEquipmentIndex] [tinyint] NOT NULL,
	[PetEquipmentItemSerial] [bigint] NULL,
	[SkinColor] [int] NULL,
	[SkinColor2] [int] NULL,
	[SkillID1] [int] NULL,
	[SkillID2] [int] NULL,
	[PetName] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
	[PetExp] [int] NULL,
	[ExpandSkillFlag] [bit] NULL,
	[DegreeOfHunger] [smallint] NOT NULL,
	[LastHungerModifyDate] [datetime2](0) NULL,
 CONSTRAINT [PK_PetEquipments] PRIMARY KEY CLUSTERED 
(
	[PetItemSerial] ASC,
	[PetEquipmentIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UNF_PetEquipments_PetEquipmentItemSerial]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_PetEquipments_PetEquipmentItemSerial] ON [dbo].[PetEquipments]
(
	[PetEquipmentItemSerial] ASC
)
WHERE ([PetEquipmentItemSerial] IS NOT NULL)
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[PetEquipments] ADD  CONSTRAINT [DF_PetEquipments_DegreeOfHunger]  DEFAULT ((0)) FOR [DegreeOfHunger]
GO
ALTER TABLE [dbo].[PetEquipments] ADD  CONSTRAINT [DF_PetEquipments_LastHungerModifyDate]  DEFAULT (getdate()) FOR [LastHungerModifyDate]
GO
