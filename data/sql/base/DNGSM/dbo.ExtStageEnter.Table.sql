USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtStageEnter]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtStageEnter](
	[id] [int] NULL,
	[_LvlMin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_LvlMax] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AbyssLvlMin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AbyssLvlMax] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_AbyssQuestID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PartyOneNumMin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_PartyOneNumMax] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MaxUsableCoin] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanEasyLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanNormalLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanHardLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanVeryHardLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CanNightmareLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_HardOpenRank] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_VeryHardOpenRank] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendEasyLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendNormalLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendHardLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendVeryHardLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendNightmareLevel] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendEasyPartyCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendNormalPartyCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendHardPartyCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendVeryHardPartyCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_RecommendNightmarePartyCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedItemCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex3] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex4] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex5] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex6] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex7] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex8] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex9] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_JewelIndex10] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Vehicle] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NeedMission] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ShareCountMap] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ListSort] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_StageOutDurability] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Weight_Easy] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Weight_Normal] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Weight_Hard] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Weight_VeryHard] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Weight_Nightmare] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_HellMapID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtStageEnter_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtStageEnter_id] ON [dbo].[ExtStageEnter]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
