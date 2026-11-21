USE [dnstaging]
GO
/****** Object:  Table [dbo].[GameMoneyTrade]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[GameMoneyTrade](
	[TradeDate] [datetime] NULL,
	[GameMoneyTradeKey] [varchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GameMoneyAmount] [bigint] NOT NULL,
	[IncomeDate] [datetime] NOT NULL,
	[ToCharacterID] [bigint] NULL,
	[PaidCashAmount] [int] NULL,
	[TotalCashAmount] [int] NULL
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NC_GameMoneyTrade_TradeDate]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_GameMoneyTrade_TradeDate] ON [dbo].[GameMoneyTrade]
(
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
