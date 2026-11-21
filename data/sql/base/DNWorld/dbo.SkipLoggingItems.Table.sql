USE [dnworld]
GO
/****** Object:  Table [dbo].[SkipLoggingItems]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SkipLoggingItems](
	[ItemID] [int] NOT NULL,
	[ItemLogCode] [tinyint] NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_UC_SkipLoggingItems_ItemID_ItemLogCode]    Script Date: 11/14/2026 8:54:16 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_SkipLoggingItems_ItemID_ItemLogCode] ON [dbo].[SkipLoggingItems]
(
	[ItemID] ASC,
	[ItemLogCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
