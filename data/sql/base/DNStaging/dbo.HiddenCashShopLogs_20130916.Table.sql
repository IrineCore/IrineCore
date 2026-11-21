USE [dnstaging]
GO
/****** Object:  Table [dbo].[HiddenCashShopLogs_20130916]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[HiddenCashShopLogs_20130916](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[HiddenShopID] [int] NOT NULL,
	[PurchaseFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_HiddenCashShopLogs_20130916_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_HiddenCashShopLogs_20130916_LogDate] ON [dbo].[HiddenCashShopLogs_20130916]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_HiddenCashShopLogs_20130916_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_HiddenCashShopLogs_20130916_CharacterID] ON [dbo].[HiddenCashShopLogs_20130916]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[HiddenCashShopLogs_20130916]  WITH CHECK ADD  CONSTRAINT [CK_HiddenCashShopLogs_20130916_LogDate] CHECK  (([LogDate]>='2013-09-16' AND [LogDate]<'2013-09-17'))
GO
ALTER TABLE [dbo].[HiddenCashShopLogs_20130916] CHECK CONSTRAINT [CK_HiddenCashShopLogs_20130916_LogDate]
GO
