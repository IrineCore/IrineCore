USE [dnmembership]
GO
/****** Object:  Table [dbo].[CashIncome]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CashIncome]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CashIncome](
	[CashIncomeID] [int] IDENTITY(1,1) NOT NULL,
	[AccountID] [int] NOT NULL,
	[CashIncomeCode] [tinyint] NOT NULL,
	[PGCode] [tinyint] NULL,
	[PGKey] [nvarchar](128) COLLATE Chinese_PRC_CI_AS NULL,
	[CashAmount] [int] NOT NULL,
	[IncomeDate] [datetime2](0) NOT NULL
) ON [DataFG1]
END
GO
/****** Object:  Index [IX_NC_CashIncome_IncomeDate]    Script Date: 11/14/2026 8:51:03 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CashIncome]') AND name = N'IX_NC_CashIncome_IncomeDate')
CREATE CLUSTERED INDEX [IX_NC_CashIncome_IncomeDate] ON [dbo].[CashIncome]
(
	[IncomeDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_CashIncome]    Script Date: 11/14/2026 8:51:10 AM ******/
IF NOT EXISTS (SELECT * FROM sys.indexes WHERE object_id = OBJECT_ID(N'[dbo].[CashIncome]') AND name = N'PK_CashIncome')
ALTER TABLE [dbo].[CashIncome] ADD  CONSTRAINT [PK_CashIncome] PRIMARY KEY NONCLUSTERED 
(
	[CashIncomeID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_CashIncome_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashIncome]'))
ALTER TABLE [dbo].[CashIncome]  WITH CHECK ADD  CONSTRAINT [FK_Accounts_CashIncome_AccountID] FOREIGN KEY([AccountID])
REFERENCES [dbo].[Accounts] ([AccountID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_Accounts_CashIncome_AccountID]') AND parent_object_id = OBJECT_ID(N'[dbo].[CashIncome]'))
ALTER TABLE [dbo].[CashIncome] CHECK CONSTRAINT [FK_Accounts_CashIncome_AccountID]
GO
