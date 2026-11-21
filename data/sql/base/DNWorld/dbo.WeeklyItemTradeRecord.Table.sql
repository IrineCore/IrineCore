USE [dnworld]
GO
/****** Object:  Table [dbo].[WeeklyItemTradeRecord]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WeeklyItemTradeRecord](
	[CharacterID] [bigint] NOT NULL,
	[ItemID] [int] NOT NULL,
	[ItemCount] [int] NOT NULL,
	[ShopID] [int] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_UC_WeeklyItemTradeRecord_CharacterID_ItemID_ShopID]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_WeeklyItemTradeRecord_CharacterID_ItemID_ShopID] ON [dbo].[WeeklyItemTradeRecord]
(
	[CharacterID] ASC,
	[ItemID] ASC,
	[ShopID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
