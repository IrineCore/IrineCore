USE [dnworld]
GO
/****** Object:  Table [dbo].[CharacterStarterItems]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[CharacterStarterItems](
	[CharacterStarterItemID] [int] IDENTITY(1,1) NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemLocationCode] [tinyint] NOT NULL,
	[ItemLocationIndex] [tinyint] NOT NULL,
	[ProvideStartDate] [smalldatetime] NULL,
	[ProvideEndDate] [smalldatetime] NULL,
	[RegisterDate] [smalldatetime] NULL,
	[ItemCount] [smallint] NOT NULL,
	[ItemDurability] [smallint] NOT NULL,
	[RandomSeed] [int] NOT NULL,
	[CoolTime] [int] NOT NULL,
	[ItemLevel] [tinyint] NOT NULL,
	[ItemPotential] [tinyint] NOT NULL,
	[SoulBoundFlag] [bit] NOT NULL,
	[SealCount] [tinyint] NOT NULL,
	[ItemOption] [tinyint] NOT NULL,
	[ItemLifespan] [int] NULL,
	[EternityFlag] [bit] NOT NULL,
 CONSTRAINT [PK_CharacterStarterItems] PRIMARY KEY CLUSTERED 
(
	[CharacterStarterItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_CharacterStarterItems_ItemID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterStarterItems_ItemID] ON [dbo].[CharacterStarterItems]
(
	[ItemID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NN_CharacterStarterItems_ProvideStartDate_ProvideEndDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_CharacterStarterItems_ProvideStartDate_ProvideEndDate] ON [dbo].[CharacterStarterItems]
(
	[ProvideStartDate] ASC,
	[ProvideEndDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
