USE [dnworld]
GO
/****** Object:  Table [dbo].[GameMoneyTrade]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[GameMoneyTrade](
	[GameMoneyTradeKey] [varchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[GameMoneyAmount] [bigint] NOT NULL,
	[IncomeDate] [datetime] NOT NULL,
	[GameMoneyTradeStatusCode] [tinyint] NOT NULL,
	[ToCharacterID] [bigint] NULL,
	[TradeDate] [datetime] NULL,
	[PaidCashAmount] [int] NULL,
	[TotalCashAmount] [int] NULL,
 CONSTRAINT [PK_GameMoneyTrade] PRIMARY KEY CLUSTERED 
(
	[GameMoneyTradeKey] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING OFF
GO
/****** Object:  Index [IX_NN_GameMoneyTrade_TradeDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GameMoneyTrade_TradeDate] ON [dbo].[GameMoneyTrade]
(
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
