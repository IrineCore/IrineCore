USE [dnworld]
GO
/****** Object:  Table [dbo].[AutoTradeResults]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AutoTradeResults](
	[TradeID] [int] NOT NULL,
	[ItemID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[UnitPrice] [int] NOT NULL,
	[UsedBudget] [int] NOT NULL,
	[TradeDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_AutoTradeResults] PRIMARY KEY CLUSTERED 
(
	[TradeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_AutoTradeResults_TradeDate]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_AutoTradeResults_TradeDate] ON [dbo].[AutoTradeResults]
(
	[TradeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
