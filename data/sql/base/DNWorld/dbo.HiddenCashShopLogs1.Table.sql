USE [dnworld]
GO
/****** Object:  Table [dbo].[HiddenCashShopLogs1]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[HiddenCashShopLogs1](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[HiddenShopID] [int] NOT NULL,
	[PurchaseFlag] [bit] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_HiddenCashShopLogs1_LogDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_HiddenCashShopLogs1_LogDate] ON [dbo].[HiddenCashShopLogs1]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
