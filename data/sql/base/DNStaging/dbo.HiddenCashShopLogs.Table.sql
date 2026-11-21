USE [dnstaging]
GO
/****** Object:  Table [dbo].[HiddenCashShopLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[HiddenCashShopLogs](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[HiddenShopID] [int] NOT NULL,
	[PurchaseFlag] [bit] NOT NULL
) ON [PRIMARY]
GO
