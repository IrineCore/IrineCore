USE [dnstaging]
GO
/****** Object:  Table [dbo].[PetalIncome]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PetalIncome](
	[PetalIncomeID] [bigint] NOT NULL,
	[AccountID] [bigint] NOT NULL,
	[PetalIncomeCode] [tinyint] NOT NULL,
	[PurchaseOrderID] [bigint] NULL,
	[AuditLogID] [int] NULL,
	[PetalAmount] [int] NOT NULL,
	[IncomeDate] [datetime2](0) NOT NULL,
	[PetalIncomeFKey] [bigint] NULL,
 CONSTRAINT [PK_PetalIncome] PRIMARY KEY CLUSTERED 
(
	[PetalIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalIncome_AccountID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalIncome_AccountID] ON [dbo].[PetalIncome]
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalIncome_IncomeDate]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalIncome_IncomeDate] ON [dbo].[PetalIncome]
(
	[IncomeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PetalIncome_PurchaseOrderID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PetalIncome_PurchaseOrderID] ON [dbo].[PetalIncome]
(
	[PurchaseOrderID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
