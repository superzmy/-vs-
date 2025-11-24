#include <iostream>
#include <random>
#include <string>
#include <map>
#include <chrono>
#include <windows.h>
using namespace std;

/**
 * 三国杀：界孙权 vs 自定义万血武将 模拟器
 * 胜利条件：
 *   孙权：压制自定义武将杀+闪<=20连续20000回合
 *   自定义武将：杀+闪>20
 */

enum CardType {
    // 杀的分类
    SHA_FIRE,      // 火杀
    SHA_THUNDER,   // 雷杀
    SHA_RED,       // 非火杀的红色杀
    SHA_BLACK,     // 黑杀
    SHAN,          // 闪
    TAO,           // 桃（酒视为桃）
    // 锦囊牌
    GUOHE,         // 过河拆桥
    SHUNSHOU,      // 顺手牵羊
    JUEDOU,        // 决斗
    JIEDAO,        // 借刀杀人
    WUZHONG,       // 无中生有
    WUXIE,         // 无懈可击
    TIESUO,        // 铁索连环
    HUOGONG,       // 火攻
    NANMAN,        // 南蛮入侵
    WANJIAN,       // 万箭齐发
    LEBU,          // 乐不思蜀
    BINGLIANG,     // 兵粮寸断
    WUGU,          // 五谷丰登
    TAOYUAN,       // 桃园结义
    SHANDIAN,      // 闪电
    // 装备牌
    WEAPON,        // 武器（除寒冰剑）
    ARMOR,         // 防具
    HORSE_PLUS,    // +1马
    HORSE_MINUS,   // -1马
    MUNIU,         // 木牛流马
    HANBING,       // 寒冰剑
    NONE
};

// 玩家
struct Player {
    string name;
    int hp;
    int hand[30];  // 手牌数组
    // 装备区
    bool hasHanbing = false;
    bool hasMuniu = false;
    bool hasHorsePlus = false;
    bool hasHorseMinus = false;
    bool hasWeapon = false;
    bool hasArmor = false;
    // 延时锦囊
    bool hasLebu = false;      // 乐不思蜀
    bool hasBingliang = false; // 兵粮寸断
    bool hasShandian = false;  // 闪电
    int usedShaThisTurn = 0;   // 本回合已出杀次数

    Player(string n, int h) : name(n), hp(h) {
        for (int i = 0; i < 30; i++) hand[i] = 0;
    }

    int totalCards() const {
        int sum = 0;
        for (int i = 0; i < 30; i++) sum += hand[i];
        return sum;
    }

    int count(CardType t) const {
        if (t >= 0 && t < 30) return hand[t];
        return 0;
    }

    void addCard(CardType t) {
        if (t != NONE && t >= 0 && t < 30) hand[t]++;
    }

    void removeCard(CardType t) {
        if (t >= 0 && t < 30 && hand[t] > 0) {
            hand[t]--;
        }
    }

    int getShaCount() const {
        return count(SHA_FIRE) + count(SHA_THUNDER) + count(SHA_RED) + count(SHA_BLACK);
    }

    int getHandLimit() const {
        return hp + (hasMuniu ? 5 : 0);
    }

    void showHand() const {
        cout << name << " 手牌: ";
        for (int i = 0; i < 30; i++) {
            if (hand[i] > 0) {
                cout << "[" << cardName((CardType)i) << "×" << hand[i] << "] ";
            }
        }
        cout << "(共" << totalCards() << "张)";
        if (hasHanbing) cout << " [装备:寒冰剑]";
        if (hasMuniu) cout << " [装备:木牛流马]";
        if (hasHorsePlus) cout << " [装备:+1马]";
        if (hasHorseMinus) cout << " [装备:-1马]";
        if (hasWeapon) cout << " [装备:武器]";
        if (hasArmor) cout << " [装备:防具]";
        if (hasLebu) cout << " [延时:乐]";
        if (hasBingliang) cout << " [延时:兵粮]";
        if (hasShandian) cout << " [延时:闪电]";
        cout << endl;
    }

    static string cardName(CardType t) {
        switch (t) {
        case SHA_FIRE: return "火杀";
        case SHA_THUNDER: return "雷杀";
        case SHA_RED: return "红杀";
        case SHA_BLACK: return "黑杀";
        case SHAN: return "闪";
        case TAO: return "桃";
        case GUOHE: return "过河拆桥";
        case SHUNSHOU: return "顺手牵羊";
        case JUEDOU: return "决斗";
        case JIEDAO: return "借刀杀人";
        case WUZHONG: return "无中生有";
        case WUXIE: return "无懈可击";
        case TIESUO: return "铁索连环";
        case HUOGONG: return "火攻";
        case NANMAN: return "南蛮入侵";
        case WANJIAN: return "万箭齐发";
        case LEBU: return "乐不思蜀";
        case BINGLIANG: return "兵粮寸断";
        case WUGU: return "五谷丰登";
        case TAOYUAN: return "桃园结义";
        case SHANDIAN: return "闪电";
        case WEAPON: return "武器";
        case ARMOR: return "防具";
        case HORSE_PLUS: return "+1马";
        case HORSE_MINUS: return "-1马";
        case MUNIU: return "木牛流马";
        case HANBING: return "寒冰剑";
        default: return "?";
        }
    }
};

// 牌堆管理
struct Deck {
    int pool[30];    // 牌堆
    int discard[30]; // 弃牌堆

    void init() {
        for (int i = 0; i < 30; i++) {
            pool[i] = 0;
            discard[i] = 0;
        }
        // 按题意初始化牌堆（扣除寒冰剑和木牛流马）
        pool[SHA_FIRE] = 5;
        pool[SHA_THUNDER] = 9;
        pool[SHA_RED] = 9;        // 红桃杀3张 + 方块杀6张
        pool[SHA_BLACK] = 21;     // 普通黑杀：黑桃7+梅花14
        pool[SHAN] = 24;
        pool[TAO] = 12 + 5;           // 桃（酒已删除）
        pool[GUOHE] = 6;
        pool[SHUNSHOU] = 5;
        pool[JUEDOU] = 3;
        pool[JIEDAO] = 2;
        pool[WUZHONG] = 4;
        pool[WUXIE] = 7;
        pool[TIESUO] = 6;
        pool[HUOGONG] = 3;
        pool[NANMAN] = 3;
        pool[WANJIAN] = 1;
        pool[LEBU] = 3;
        pool[BINGLIANG] = 2;
        pool[WUGU] = 2;
        pool[TAOYUAN] = 1;
        pool[SHANDIAN] = 2;
        pool[WEAPON] = 12;        // 13张武器 - 1寒冰剑
        pool[ARMOR] = 6;
        pool[HORSE_PLUS] = 3;
        pool[HORSE_MINUS] = 4;
        // MUNIU和HANBING不在牌堆，已给孙权装备
    }

    CardType drawCard(mt19937& rng) {
        if (total() == 0) reshuffle();
        int totalCards = total();
        if (totalCards == 0) return NONE;
        
        uniform_int_distribution<int> dist(1, totalCards);
        int r = dist(rng), cum = 0;
        for (int i = 0; i < 30; i++) {
            cum += pool[i];
            if (r <= cum && pool[i] > 0) {
                pool[i]--;
                return (CardType)i;
            }
        }
        return NONE;
    }

    void discardCard(CardType c) {
        if (c != NONE && c >= 0 && c < 30) {
            discard[c]++;
        }
    }

    int total() {
        int s = 0;
        for (int i = 0; i < 30; i++) s += pool[i];
        return s;
    }
    int totalDiscard() {
        int s = 0;
        for (int i = 0; i < 30; i++) s += discard[i];
        return s;
    }
    void reshuffle();
};

Deck deck;
Player sq("界孙权", 4), cus("自定义武将", 10000);

void Deck::reshuffle()
{
    for (int i = 0; i < 30; i++) {
        pool[i] += discard[i];
        discard[i] = 0;
    }
    cout << "[洗牌] 弃牌堆→摸牌堆, 当前牌堆=" << total() << " 手牌=[孙权]" << sq.totalCards() << "+[自定义]" << cus.totalCards() << "张,装备有[孙权]"
        << sq.hasHanbing + sq.hasWeapon + sq.hasArmor + sq.hasHorsePlus + sq.hasHorseMinus + sq.hasMuniu << "+[自定义]"
        << cus.hasHanbing + cus.hasWeapon + cus.hasArmor + cus.hasHorsePlus + cus.hasHorseMinus + cus.hasMuniu
        << "挂载状态牌有" << sq.hasBingliang + sq.hasLebu + sq.hasShandian << "+" << cus.hasBingliang + cus.hasLebu + cus.hasShandian
        << endl;
}

// 工具函数：随机删除一张手牌
CardType randomRemove(Player& p, mt19937& rng) {
    int total = p.totalCards();
    if (total == 0) return NONE;
    
    uniform_int_distribution<int> d(1, total);
    int r = d(rng), cur = 0;
    
    for (int i = 0; i < 30; i++) {
        cur += p.hand[i];
        if (r <= cur && p.hand[i] > 0) {
            p.removeCard((CardType)i);
            return (CardType)i;
        }
    }
    return NONE;
}

// 工具函数：删除一张杀
CardType removeSha(Player& p) {
    if (p.count(SHA_FIRE) > 0) { p.removeCard(SHA_FIRE); return SHA_FIRE; }
    if (p.count(SHA_THUNDER) > 0) { p.removeCard(SHA_THUNDER); return SHA_THUNDER; }
    if (p.count(SHA_RED) > 0) { p.removeCard(SHA_RED); return SHA_RED; }
    if (p.count(SHA_BLACK) > 0) { p.removeCard(SHA_BLACK); return SHA_BLACK; }
    return NONE;
}

// 弃牌阶段优先级选择（不随机）
CardType pickDiscardForPhase(Player& p, int oppCards) {
    CardType c = NONE;
    // 1. 多余的闪（保留3张）
    if (c == NONE && p.count(SHAN) > 3) { c = SHAN; p.removeCard(c); }
    // 2. 多余的杀（保留3张）
    if (c == NONE && p.getShaCount() > 3) { c = removeSha(p); }
    // 3. 武器
    if (c == NONE && p.count(WEAPON) > 0) { c = WEAPON; p.removeCard(c); }
    // 4. 防具（已装备）
    if (c == NONE && p.count(ARMOR) > 0 && p.hasArmor) { c = ARMOR; p.removeCard(c); }
    // 5. +1马（已装备）
    if (c == NONE && p.count(HORSE_PLUS) > 0 && p.hasHorsePlus) { c = HORSE_PLUS; p.removeCard(c); }
    // 6. -1马（已装备）
    if (c == NONE && p.count(HORSE_MINUS) > 0 && p.hasHorseMinus) { c = HORSE_MINUS; p.removeCard(c); }
    // 7. 乐不思蜀
    if (c == NONE && p.count(LEBU) > 0) { c = LEBU; p.removeCard(c); }
    // 8. 闪电
    if (c == NONE && p.count(SHANDIAN) > 0) { c = SHANDIAN; p.removeCard(c); }
    // 9. 兵粮寸断
    if (c == NONE && p.count(BINGLIANG) > 0) { c = BINGLIANG; p.removeCard(c); }
    // 10. 桃园结义
    if (c == NONE && p.count(TAOYUAN) > 0) { c = TAOYUAN; p.removeCard(c); }
    // 11. 过河拆桥（仅当对手无牌）
    if (c == NONE && p.count(GUOHE) > 0 && oppCards == 0) { c = GUOHE; p.removeCard(c); }
    // 12. 顺手牵羊（仅当对手无牌）
    if (c == NONE && p.count(SHUNSHOU) > 0 && oppCards == 0) { c = SHUNSHOU; p.removeCard(c); }
    // 13. 火攻
    if (c == NONE && p.count(HUOGONG) > 0) { c = HUOGONG; p.removeCard(c); }
    // 14. 借刀杀人
    if (c == NONE && p.count(JIEDAO) > 0) { c = JIEDAO; p.removeCard(c); }
    // 15. 决斗
    if (c == NONE && p.count(JUEDOU) > 0) { c = JUEDOU; p.removeCard(c); }
    // 16. 五谷丰登
    if (c == NONE && p.count(WUGU) > 0) { c = WUGU; p.removeCard(c); }
    // 17. 南蛮入侵
    if (c == NONE && p.count(NANMAN) > 0) { c = NANMAN; p.removeCard(c); }
    // 18. 万箭齐发
    if (c == NONE && p.count(WANJIAN) > 0) { c = WANJIAN; p.removeCard(c); }
    // 19. 多余的闪（保留2张）
    if (c == NONE && p.count(SHAN) > 2) { c = SHAN; p.removeCard(c); }
    // 20. 多余的杀（保留2张）
    if (c == NONE && p.getShaCount() > 2) { c = removeSha(p); }
    // 21. 铁索连环
    if (c == NONE && p.count(TIESUO) > 0) { c = TIESUO; p.removeCard(c); }
    // 22. 桃（生命满时）
    if (c == NONE && p.count(TAO) > 0 && p.hp >= 4) { c = TAO; p.removeCard(c); }
    // 23. 闪
    if (c == NONE && p.count(SHAN) > 0) { c = SHAN; p.removeCard(c); }
    // 24. 杀
    if (c == NONE && p.getShaCount() > 0) { c = removeSha(p); }
    // 25. 防具
    if (c == NONE && p.count(ARMOR) > 0) { c = ARMOR; p.removeCard(c); }
    // 26. 桃
    if (c == NONE && p.count(TAO) > 0) { c = TAO; p.removeCard(c); }
    
    // 兜底（不随机）：先丢除无懈/无中之外的任意牌
    if (c == NONE) {
        for (int i = 0; i < 30; ++i) {
            if (i == WUXIE || i == WUZHONG) continue;
            if (p.hand[i] > 0) { p.removeCard((CardType)i); return (CardType)i; }
        }
    }
    // 最后不得不丢：无懈可击，再其次无中生有
    if (c == NONE && p.count(WUXIE) > 0) { p.removeCard(WUXIE); return WUXIE; }
    if (c == NONE && p.count(WUZHONG) > 0) { p.removeCard(WUZHONG); return WUZHONG; }
    
    return c;
}

bool nullifyContest(Player& attacker, Player& defender, Deck& deck) {
    int a = attacker.count(WUXIE);
    int d = defender.count(WUXIE);
    cout << "[无懈对抗] 自定义" << a << " vs 孙权" << d;
    if (d > a) {
        for (int i = 0; i < a; ++i) {
            attacker.removeCard(WUXIE); deck.discardCard(WUXIE);
            defender.removeCard(WUXIE); deck.discardCard(WUXIE);
        }
        defender.removeCard(WUXIE); deck.discardCard(WUXIE);
        cout << " → 孙权防住" << endl;
        return true;
    } else {
        for (int i = 0; i < d; ++i) {
            attacker.removeCard(WUXIE); deck.discardCard(WUXIE);
            defender.removeCard(WUXIE); deck.discardCard(WUXIE);
        }
        cout << " → 破防" << endl;
        return false;
    }
}

int main() {
    // 设置控制台编码为UTF-8
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
    
    ios::sync_with_stdio(false);
    mt19937 rng((unsigned)chrono::system_clock::now().time_since_epoch().count());
    uniform_real_distribution<double> prob(0.0, 1.0);

    deck.init();

    // 初始装备
    sq.hasHanbing = true;
    sq.hasMuniu = true;

    // 初始手牌：孙权9张随机牌，自定义武将4张
    for (int i = 0; i < 9; i++) sq.addCard(deck.drawCard(rng));
    for (int i = 0; i < 4; i++) cus.addCard(deck.drawCard(rng));

    cout << "=== 开局 ===" << endl;
    sq.showHand();
    cus.showHand();

    int maxTurns = 20000;
    for (int turn = 1; turn <= maxTurns; ++turn) {
        cout << "\n========== 第 " << turn << " 回合 ==========" << endl;
        
        cout << "[牌堆] 摸牌堆=" << deck.total() << " 弃牌堆 = " << deck.totalDiscard() << " 手牌 = [孙权]" << sq.totalCards() << " + [自定义]" << cus.totalCards() << "张, 装备有[孙权]"
            << sq.hasHanbing + sq.hasWeapon + sq.hasArmor + sq.hasHorsePlus + sq.hasHorseMinus + sq.hasMuniu << "+[自定义]"
            << cus.hasHanbing + cus.hasWeapon + cus.hasArmor + cus.hasHorsePlus + cus.hasHorseMinus + cus.hasMuniu
            << "挂载状态牌有" << sq.hasBingliang + sq.hasLebu + sq.hasShandian << "+" << cus.hasBingliang + cus.hasLebu + cus.hasShandian
            << endl;
        if (deck.total() + sq.totalCards() + cus.totalCards() + deck.totalDiscard() +
            sq.hasHanbing + sq.hasWeapon + sq.hasArmor + sq.hasHorsePlus + sq.hasHorseMinus + sq.hasMuniu +
            cus.hasHanbing + cus.hasWeapon + cus.hasArmor + cus.hasHorsePlus + cus.hasHorseMinus + cus.hasMuniu +
            sq.hasBingliang + sq.hasLebu + sq.hasShandian + cus.hasBingliang + cus.hasLebu + cus.hasShandian
            != 162
            )
        {
            __debugbreak();
        }

        sq.usedShaThisTurn = 0;
        
        // === 孙权回合开始 ===
        if (sq.hasShandian && prob(rng) < 0.15) {
            cout << "[闪电] 孙权被闪电击中! -3 HP" << endl;
            sq.hp -= 3;
            sq.hasShandian = false;
            deck.discardCard(SHANDIAN);
            if (sq.hp <= 0) {
                cout << "孙权阵亡,自定义武将胜利!" << endl;
                break;
            }
        }
        
        bool skipPlayPhase = false;
        if (sq.hasLebu) {
            cout << "[乐不思蜀] 孙权被乐限制";
            // 生命满时不使用无懈可击
            if (sq.count(WUXIE) > 0 && sq.hp < 4) {
                sq.removeCard(WUXIE);
                deck.discardCard(WUXIE);
                cout << ", 用无懈抵消" << endl;
            } else {
                cout << ", 跳过出牌阶段" << endl;
                skipPlayPhase = true;
            }
            sq.hasLebu = false;
            deck.discardCard(LEBU);
        }
        
        if (!sq.hasBingliang) {
            CardType c1 = deck.drawCard(rng), c2 = deck.drawCard(rng);
            sq.addCard(c1); sq.addCard(c2);
            cout << "[摸牌] 孙权: " << Player::cardName(c1) << ", " << Player::cardName(c2) << endl;
        } else {
            cout << "[兵粮] 孙权跳过摸牌" << endl;
            sq.hasBingliang = false;
            deck.discardCard(BINGLIANG);
        }
        
        if (!skipPlayPhase) {
            for (int repeat = 0; repeat < 6; ++repeat)
            {
                // 铁索
                while (sq.count(TIESUO) > 0) {
                    sq.removeCard(TIESUO); deck.discardCard(TIESUO);
                    CardType c = deck.drawCard(rng); sq.addCard(c);
                    cout << "[铁索] 孙权摸: " << Player::cardName(c) << endl;
                }

                // 无中生有
                while (sq.count(WUZHONG) > 0) {
                    sq.removeCard(WUZHONG); deck.discardCard(WUZHONG);
                    CardType c1 = deck.drawCard(rng), c2 = deck.drawCard(rng);
                    sq.addCard(c1); sq.addCard(c2);
                    cout << "[无中] 孙权摸: " << Player::cardName(c1) << ", " << Player::cardName(c2) << endl;
                }

                // 五谷
                while (sq.count(WUGU) > 0) {
                    sq.removeCard(WUGU); deck.discardCard(WUGU);
                    CardType c = deck.drawCard(rng); sq.addCard(c);
                    cout << "[五谷] 孙权摸: " << Player::cardName(c) << endl;
                }

                // 顺手
                while (sq.count(SHUNSHOU) > 0 && (cus.totalCards() > 0 || sq.hasShandian)) {
                    sq.removeCard(SHUNSHOU); deck.discardCard(SHUNSHOU);
                    if (sq.hasShandian) {
                        sq.hasShandian = false;
                        sq.addCard(SHANDIAN);
                        cout << "[顺手] 孙权夺: 自定义闪电" << endl;
                    }
                    else
                    {
                        CardType s = randomRemove(cus, rng);
                        sq.addCard(s);
                        cout << "[顺手] 孙权夺: " << Player::cardName(s) << endl;
                    }
                }

                // 借刀
                while (sq.count(JIEDAO) > 0 && cus.hasWeapon) {
                    sq.removeCard(JIEDAO); deck.discardCard(JIEDAO);
                    cus.hasWeapon = false;
                    sq.addCard(WEAPON);
                    cout << "[借刀] 自定义被夺武器" << endl;
                }

                // 装备
                while (sq.count(MUNIU) > 0 && !sq.hasMuniu) {
                    sq.removeCard(MUNIU); sq.hasMuniu = true;
                    cout << "[装备] 孙权装备木牛" << endl;
                }
                while (sq.count(HANBING) > 0 && !sq.hasHanbing) {
                    sq.removeCard(HANBING); sq.hasHanbing = true;
                    cout << "[装备] 孙权装备寒冰剑" << endl;
                    if (sq.hasWeapon)
                    {
                        sq.hasWeapon = false;
                        deck.discardCard(WEAPON);
                        cout << "[装备] 孙权丢弃武器" << endl;
                    }
                }
                while (sq.count(HORSE_PLUS) > 0 && !sq.hasHorsePlus) {
                    sq.removeCard(HORSE_PLUS); sq.hasHorsePlus = true;
                    cout << "[装备] 孙权装备+1马" << endl;
                }
                while (sq.count(HORSE_MINUS) > 0 && !sq.hasHorseMinus) {
                    sq.removeCard(HORSE_MINUS); sq.hasHorseMinus = true;
                    cout << "[装备] 孙权装备-1马" << endl;
                }
                while (sq.count(WEAPON) > 0 && !sq.hasWeapon && !sq.hasHanbing) {
                    sq.removeCard(WEAPON); sq.hasWeapon = true;
                    cout << "[装备] 孙权装备武器" << endl;
                }
                while (sq.count(ARMOR) > 0 && !sq.hasArmor) {
                    sq.removeCard(ARMOR); sq.hasArmor = true;
                    cout << "[装备] 孙权装备防具" << endl;
                }

                // 桃
                while (sq.count(TAO) > 0 && sq.hp < 4) {
                    sq.removeCard(TAO); deck.discardCard(TAO); sq.hp++;
                    cout << "[桃] 孙权回复HP至 " << sq.hp << endl;
                }
                while (sq.count(TAOYUAN) > 0 && sq.hp < 4) {
                    sq.removeCard(TAOYUAN); deck.discardCard(TAOYUAN); sq.hp++;
                    cout << "[桃园] 孙权回复HP至 " << sq.hp << endl;
                }

                // 过河>2
                while (sq.count(GUOHE) > 0 && (cus.totalCards() > 0 || sq.hasShandian)) {
                    sq.removeCard(GUOHE); deck.discardCard(GUOHE);
                    if (sq.hasShandian) {
                        sq.hasShandian = false;
                        deck.discardCard(SHANDIAN);
                        cout << "[过河] 孙权拆: 自定义闪电" << endl;
                    }
                    else
                    {
                        CardType r = randomRemove(cus, rng);
                        deck.discardCard(r);
                        cout << "[过河] 孙权拆: " << Player::cardName(r) << endl;
                    }
                }


                // 兵粮
                if (!cus.hasBingliang && sq.count(BINGLIANG) > 0) {
                    sq.removeCard(BINGLIANG); cus.hasBingliang = true;
                    cout << "[兵粮] 孙权对自定义使用" << endl;
                }

                // 出杀
                if (sq.hasHanbing && sq.getShaCount() > 0 && sq.usedShaThisTurn == 0) {
                    CardType sha = removeSha(sq); deck.discardCard(sha);
                    sq.usedShaThisTurn++;
                    cout << "[寒冰杀] 孙权出" << Player::cardName(sha);
                    if (cus.count(SHAN) > 0) {
                        int before = cus.count(SHAN);
                        cus.removeCard(SHAN); deck.discardCard(SHAN);
                        cout << " → 自定义打出闪(闪" << before << "→" << cus.count(SHAN) << ")" << endl;
                    }
                    else {
                        cout << " → 自定义无闪,";
                        int removed = 0;
                        if (removed < 2 && cus.hasHorseMinus) { cus.hasHorseMinus = false; removed++; deck.discardCard(HORSE_MINUS); cout << " 失-1马"; }
                        if (removed < 2 && cus.hasArmor) { cus.hasArmor = false; removed++; deck.discardCard(ARMOR); cout << " 失防具"; }
                        while (removed < 2 && cus.totalCards() > 0) {
                            CardType r = randomRemove(cus, rng); deck.discardCard(r);
                            removed++; cout << " 失" << Player::cardName(r);
                        }
                        cout << endl;
                    }
                }
            }

            sq.showHand();
            // 制衡
            int oldTotal = sq.totalCards();
            int toDiscard = max(0, sq.totalCards() - min(sq.count(WUXIE) + sq.count(SHUNSHOU) + sq.count(GUOHE), sq.getHandLimit()));
            if (toDiscard > 0) {
                cout << "[制衡] 弃" << toDiscard << "张: ";
                // 智能弃牌：优先级（从高到低）
                for (int i = 0; i < toDiscard; i++) {
                    CardType c = NONE;
                    int oppCards = cus.totalCards();
                    
                    // 16. 五谷丰登
                    if (c == NONE && sq.count(WUGU) > 0) { c = WUGU; sq.removeCard(c); }
                    // 23. 铁索连环
                    if (c == NONE && sq.count(TIESUO) > 0) { c = TIESUO; sq.removeCard(c); }
                    // 17. 南蛮入侵
                    if (c == NONE && sq.count(NANMAN) > 0) { c = NANMAN; sq.removeCard(c); }
                    // 18. 万箭齐发
                    if (c == NONE && sq.count(WANJIAN) > 0) { c = WANJIAN; sq.removeCard(c); }
                    // 3. 武器
                    if (c == NONE && sq.count(WEAPON) > 0) { c = WEAPON; sq.removeCard(c); }
                    // 4. 防具（已装备）
                    if (c == NONE && sq.count(ARMOR) > 0 && sq.hasArmor) { c = ARMOR; sq.removeCard(c); }
                    // 5. +1马（已装备）
                    if (c == NONE && sq.count(HORSE_PLUS) > 0 && sq.hasHorsePlus) { c = HORSE_PLUS; sq.removeCard(c); }
                    // 7. 乐不思蜀
                    if (c == NONE && sq.count(LEBU) > 0) { c = LEBU; sq.removeCard(c); }
                    // 10. 桃园结义
                    if (c == NONE && sq.count(TAOYUAN) > 0) { c = TAOYUAN; sq.removeCard(c); }
                    // 6. -1马（已装备）
                    if (c == NONE && sq.count(HORSE_MINUS) > 0 && sq.hasHorseMinus) { c = HORSE_MINUS; sq.removeCard(c); }
                    // 8. 闪电
                    if (c == NONE && sq.count(SHANDIAN) > 0) { c = SHANDIAN; sq.removeCard(c); }
                    // 14. 借刀杀人
                    if (c == NONE && sq.count(JIEDAO) > 0) { c = JIEDAO; sq.removeCard(c); }
                    // 13. 火攻
                    if (c == NONE && sq.count(HUOGONG) > 0) { c = HUOGONG; sq.removeCard(c); }
                    // 15. 决斗
                    if (c == NONE && sq.count(JUEDOU) > 0) { c = JUEDOU; sq.removeCard(c); }
                    // 24. 桃（生命满时）
                    if (c == NONE && sq.count(TAO) >= 1 && sq.hp >= 4) { c = TAO; sq.removeCard(c); }

                    // 1. 多余的闪（保留3张）
                    if (c == NONE && sq.count(SHAN) > 3) { c = SHAN; sq.removeCard(c); }
                    // 2. 多余的杀（保留3张）
                    if (c == NONE && sq.getShaCount() > 3) { c = removeSha(sq); }
                    // 9. 兵粮寸断
                    if (c == NONE && sq.count(BINGLIANG) > 0) { c = BINGLIANG; sq.removeCard(c); }
                    // 11. 过河拆桥（仅当对手无牌）
                    if (c == NONE && sq.count(GUOHE) > 0 && oppCards == 0) { c = GUOHE; sq.removeCard(c); }
                    // 12. 顺手牵羊（仅当对手无牌）
                    if (c == NONE && sq.count(SHUNSHOU) > 0 && oppCards == 0) { c = SHUNSHOU; sq.removeCard(c); }
                    // 19. 多余的闪（保留2张）
                    if (c == NONE && sq.count(SHAN) > 1) { c = SHAN; sq.removeCard(c); }
                    // 20. 多余的杀（保留2张）
                    if (c == NONE && sq.getShaCount() > 2) { c = removeSha(sq); }
                    // 27. 防具
                    if (c == NONE && sq.count(ARMOR) > 0) { c = ARMOR; sq.removeCard(c); }
                    // 25. 闪
                    if (c == NONE && sq.count(SHAN) > 0) { c = SHAN; sq.removeCard(c); }
                    // 26. 杀
                    if (c == NONE && sq.getShaCount() > 0) { c = removeSha(sq); }
                    // 30. 其他随机
                    if (c == NONE) { 
                        c = randomRemove(sq, rng);
                        if (c == WUXIE && sq.count(WUXIE) + sq.count(GUOHE) < min(5, sq.getHandLimit()))
                            __debugbreak();
                    }
                    
                    deck.discardCard(c);
                    cout << Player::cardName(c);
                    if (i < toDiscard - 1) cout << ", ";
                }
                cout << "; 摸" << toDiscard + 1 << "张: ";
                // 摸牌
                for (int i = 0; i < toDiscard + 1; i++) {
                    CardType c = deck.drawCard(rng);
                    sq.addCard(c);
                    cout << Player::cardName(c);
                    if (i < toDiscard) cout << ", ";
                }
                cout << endl;
            }
            
            // 出牌院2: 再次检查
            while (sq.count(TIESUO) > 0) {
                sq.removeCard(TIESUO); deck.discardCard(TIESUO);
                CardType c = deck.drawCard(rng); sq.addCard(c);
                cout << "[铁索] 孙权摸: " << Player::cardName(c) << endl;
            }
            while (sq.count(WUZHONG) > 0) {
                sq.removeCard(WUZHONG); deck.discardCard(WUZHONG);
                CardType c1 = deck.drawCard(rng), c2 = deck.drawCard(rng);
                sq.addCard(c1); sq.addCard(c2);
                cout << "[无中] 孙权摸: " << Player::cardName(c1) << ", " << Player::cardName(c2) << endl;
            }
            while (sq.count(SHUNSHOU) > 0 && cus.totalCards() > 0) {
                sq.removeCard(SHUNSHOU); deck.discardCard(SHUNSHOU);
                CardType s = randomRemove(cus, rng); 
                sq.addCard(s);
                cout << "[顺手] 孙权夺: " << Player::cardName(s) << endl;
            }
            while (sq.count(JIEDAO) > 0 && cus.hasWeapon) {
                sq.removeCard(JIEDAO); deck.discardCard(JIEDAO);
                cus.hasWeapon = false;
                sq.addCard(WEAPON);
                cout << "[借刀] 自定义失去武器" << endl;
            }
            while (sq.count(WUGU) > 0) {
                sq.removeCard(WUGU); deck.discardCard(WUGU);
                CardType c = deck.drawCard(rng); sq.addCard(c);
                cout << "[五谷] 孙权摸: " << Player::cardName(c) << endl;
            }

            // 装备
            while (sq.count(MUNIU) > 0 && !sq.hasMuniu) {
                sq.removeCard(MUNIU); sq.hasMuniu = true;
                cout << "[装备] 孙权装备木牛" << endl;
            }
            while (sq.count(HANBING) > 0 && !sq.hasHanbing) {
                sq.removeCard(HANBING); sq.hasHanbing = true;
                cout << "[装备] 孙权装备寒冰剑" << endl;
            }
            while (sq.count(HORSE_PLUS) > 0 && !sq.hasHorsePlus) {
                sq.removeCard(HORSE_PLUS); sq.hasHorsePlus = true;
                cout << "[装备] 孙权装备+1马" << endl;
            }
            while (sq.count(HORSE_MINUS) > 0 && !sq.hasHorseMinus) {
                sq.removeCard(HORSE_MINUS); sq.hasHorseMinus = true;
                cout << "[装备] 孙权装备-1马" << endl;
            }
            while (sq.count(WEAPON) > 0 && !sq.hasWeapon && !sq.hasHanbing) {
                sq.removeCard(WEAPON); sq.hasWeapon = true;
                cout << "[装备] 孙权装备武器" << endl;
            }
            while (sq.count(ARMOR) > 0 && !sq.hasArmor) {
                sq.removeCard(ARMOR); sq.hasArmor = true;
                cout << "[装备] 孙权装备防具" << endl;
            }

            // 桃
            while (sq.count(TAO) > 0 && sq.hp < 4) {
                sq.removeCard(TAO); deck.discardCard(TAO); sq.hp++;
                cout << "[桃] 孙权回复HP至 " << sq.hp << endl;
            }
            while (sq.count(TAOYUAN) > 0 && sq.hp < 4) {
                sq.removeCard(TAOYUAN); deck.discardCard(TAOYUAN); sq.hp++;
                cout << "[桃园] 孙权回复HP至 " << sq.hp << endl;
            }
            // 兵粮
            if (!cus.hasBingliang && sq.count(BINGLIANG) > 0) {
                sq.removeCard(BINGLIANG); cus.hasBingliang = true;
                cout << "[兵粮] 孙权对自定义使用" << endl;
            }
            // 出杀
            if (sq.hasHanbing && sq.getShaCount() > 0 && sq.usedShaThisTurn == 0) {
                CardType sha = removeSha(sq); deck.discardCard(sha);
                sq.usedShaThisTurn++;
                cout << "[寒冰杀] 孙权出" << Player::cardName(sha);
                if (cus.count(SHAN) > 0) {
                    int before = cus.count(SHAN);
                    cus.removeCard(SHAN); deck.discardCard(SHAN);
                    cout << " → 自定义打出闪(闪" << before << "→" << cus.count(SHAN) << ")" << endl;
                }
                else {
                    cout << " → 自定义无闪,";
                    int removed = 0;
                    if (removed < 2 && cus.hasHorseMinus) { cus.hasHorseMinus = false; removed++; deck.discardCard(HORSE_MINUS); cout << " 失-1马"; }
                    if (removed < 2 && cus.hasArmor) { cus.hasArmor = false; removed++; deck.discardCard(ARMOR); cout << " 失防具"; }
                    while (removed < 2 && cus.totalCards() > 0) {
                        CardType r = randomRemove(cus, rng); deck.discardCard(r);
                        removed++; cout << " 失" << Player::cardName(r);
                    }
                    cout << endl;
                }
            }
            while (sq.count(GUOHE) > 2 && cus.totalCards() > 0) {
                sq.removeCard(GUOHE); deck.discardCard(GUOHE);
                CardType r = randomRemove(cus, rng); deck.discardCard(r);
                cout << "[过河] 孙权拆: " << Player::cardName(r) << endl;
            }
        }
        
        if (sq.totalCards() > sq.getHandLimit())
        {
            cout << "[弃牌] 孙权弃牌: ";
            // 弃牌
            while (sq.totalCards() > sq.getHandLimit()) {
                CardType c = pickDiscardForPhase(sq, cus.totalCards());
                deck.discardCard(c);
                cout << Player::cardName(c);
                if (sq.totalCards() > sq.getHandLimit())
                    cout << ", ";
            }
            cout << endl;
        }
        
        // === 自定义回合 ===
        cout << "\n--- 自定义回合 ---" << endl;
        
        if (!cus.hasBingliang) {
            CardType c1 = deck.drawCard(rng), c2 = deck.drawCard(rng);
            cus.addCard(c1); cus.addCard(c2);
            cout << "[摸牌] 自定义: " << Player::cardName(c1) << ", " << Player::cardName(c2) << endl;
        } else {
            cout << "[兵粮] 自定义跳过摸牌" << endl;
            cus.hasBingliang = false;
            deck.discardCard(BINGLIANG);
        }
        
        // 装备
        while (cus.count(ARMOR) > 0 && !cus.hasArmor) {
            cus.removeCard(ARMOR); cus.hasArmor = true;
            cout << "[装备] 自定义装备防具" << endl;
        }
        while (cus.count(HORSE_PLUS) > 0 && !cus.hasHorsePlus) {
            cus.removeCard(HORSE_PLUS); cus.hasHorsePlus = true;
            cout << "[装备] 自定义装备+1马" << endl;
        }
        
        // 铁索
        while (cus.count(TIESUO) > 0) {
            cus.removeCard(TIESUO); deck.discardCard(TIESUO);
            CardType c = deck.drawCard(rng); cus.addCard(c);
            cout << "[铁索] 自定义摸: " << Player::cardName(c) << endl;
        }
        
        // 无中
        while (cus.count(WUZHONG) > 0 && cus.totalCards() < 20) {
            cus.removeCard(WUZHONG); deck.discardCard(WUZHONG);
            CardType c1 = deck.drawCard(rng), c2 = deck.drawCard(rng);
            cus.addCard(c1); cus.addCard(c2);
            cout << "[无中] 自定义摸: " << Player::cardName(c1) << ", " << Player::cardName(c2) << endl;
        }
        while (cus.count(WUGU) > 0) {
            cus.removeCard(WUGU); deck.discardCard(WUGU);
            CardType c = deck.drawCard(rng); cus.addCard(c);
            cout << "[五谷] 自定义摸: " << Player::cardName(c) << endl;
        }
        while (cus.count(TAOYUAN) > 0 && cus.totalCards() > 20 && sq.hp == 4) {
            cus.removeCard(TAOYUAN); deck.discardCard(TAOYUAN);
            cout << "[桃园] 自定义使用" << endl;
        }
        while (cus.count(LEBU) > 0 && !sq.hasLebu) {
            cus.removeCard(LEBU); sq.hasLebu = true;
            cout << "[乐] 自定义对孙权使用" << endl;
        }
        while (cus.count(SHANDIAN) > 0 && !sq.hasShandian) {
            cus.removeCard(SHANDIAN); sq.hasShandian = true;
            cout << "[闪电] 自定义对孙权使用" << endl;
        }
        
        // 过河/顺手/借刀
        if (cus.count(GUOHE) > 0 && (sq.hasMuniu || sq.hasHanbing)) {
            bool willUse = (cus.count(WUXIE) >= sq.count(WUXIE));
            if (willUse) {
                cus.removeCard(GUOHE); deck.discardCard(GUOHE);
                bool blocked = nullifyContest(cus, sq, deck);
                if (!blocked) {
                    if (sq.hasMuniu) {
                        sq.hasMuniu = false;
                        cout << "[过河] 自定义拆除孙权的木牛" << endl;
                        deck.discardCard(MUNIU);
                    } else if (sq.hasHanbing) {
                        sq.hasHanbing = false;
                        cout << "[过河] 自定义拆除孙权的寒冰剑" << endl;
                        deck.discardCard(HANBING);
                    }
                } else {
                    cout << "[过河] 被孙权无懈防住" << endl;
                }
            }
        }
        if (cus.count(SHUNSHOU) > 0 && cus.hasHorseMinus && (sq.hasMuniu || sq.hasHanbing)) {
            bool willUse = (cus.count(WUXIE) >= sq.count(WUXIE));
            if (willUse) {
                cus.removeCard(SHUNSHOU); deck.discardCard(SHUNSHOU);
                bool blocked = nullifyContest(cus, sq, deck);
                if (!blocked) {
                    if (sq.hasMuniu) {
                        sq.hasMuniu = false; cus.addCard(MUNIU);
                        cout << "[顺手] 自定义夺取孙权的木牛" << endl;
                    } else if (sq.hasHanbing) {
                        sq.hasHanbing = false; cus.addCard(HANBING);
                        cout << "[顺手] 自定义夺取孙权的寒冰剑" << endl;
                    }
                } else {
                    cout << "[顺手] 被孙权无懈防住" << endl;
                }
            }
        }
        if (cus.count(JIEDAO) > 0 && sq.hasHanbing) {
            bool willUse = (cus.count(WUXIE) >= sq.count(WUXIE) && sq.getShaCount() == 0);
            if (willUse) {
                cus.removeCard(JIEDAO); deck.discardCard(JIEDAO);
                bool blocked = nullifyContest(cus, sq, deck);
                if (!blocked) {
                    sq.hasHanbing = false; cus.addCard(HANBING);
                    cout << "[借刀] 自定义夺取孙权的寒冰剑" << endl;
                } else {
                    cout << "[借刀] 被孙权无懈防住" << endl;
                }
            }
        }
        
        // 攻击锦囊
        int attackCards = cus.count(NANMAN) + cus.count(WANJIAN) + cus.count(JUEDOU) + cus.count(HUOGONG);
        if (attackCards >= 2) {
            while (cus.count(NANMAN) > 0) {
                cus.removeCard(NANMAN); deck.discardCard(NANMAN);
                cout << "[南蛮] 自定义使用";
                // 优先打杀，只有当没有杀且生命<=2时才用无懈
                if (sq.getShaCount() > 0) {
                    CardType sha = removeSha(sq); deck.discardCard(sha);
                    cout << " → 孙权打出" << Player::cardName(sha) << endl;
                } else if ((sq.count(WUXIE) > 0 && sq.hp <= 2)
                    || sq.count(WUXIE) >= 3) {
                    sq.removeCard(WUXIE); deck.discardCard(WUXIE);
                    cout << " → 孙权无懈" << endl;
                } else {
                    sq.hp--;
                    cout << " → 孙权受伤,HP=" << sq.hp << endl;
                    if (sq.hp <= 0) { cout << "孙权阵亡!" << endl; goto END; }
                }
            }
            while (cus.count(WANJIAN) > 0) {
                cus.removeCard(WANJIAN); deck.discardCard(WANJIAN);
                cout << "[万箭] 自定义使用";
                // 优先打闪，只有当没有闪且生命<=2时才用无懈
                if (sq.count(SHAN) > 0) {
                    sq.removeCard(SHAN); deck.discardCard(SHAN);
                    cout << " → 孙权打出闪" << endl;
                } else if ((sq.count(WUXIE) > 0 && sq.hp <= 2)
                    || sq.count(WUXIE) >= 3) {
                    sq.removeCard(WUXIE); deck.discardCard(WUXIE);
                    cout << " → 孙权无懈" << endl;
                } else {
                    sq.hp--;
                    cout << " → 孙权受伤,HP=" << sq.hp << endl;
                    if (sq.hp <= 0) { cout << "孙权阵亡!" << endl; goto END; }
                }
            }
            while (cus.count(JUEDOU) > 0) {
                cus.removeCard(JUEDOU); deck.discardCard(JUEDOU);
                cout << "[决斗] 自定义使用";
                // 优先打杀，只有当没有杀且生命<=2时才用无懈
                if (sq.getShaCount() > 0) {
                    CardType sha = removeSha(sq); deck.discardCard(sha);
                    cout << " → 孙权打出" << Player::cardName(sha) << endl;
                } else if ((sq.count(WUXIE) > 0 && sq.hp <= 2)
                    || sq.count(WUXIE) >= 3) {
                    sq.removeCard(WUXIE); deck.discardCard(WUXIE);
                    cout << " → 孙权无懈" << endl;
                } else {
                    sq.hp--;
                    cout << " → 孙权受伤,HP=" << sq.hp << endl;
                    if (sq.hp <= 0) { cout << "孙权阵亡!" << endl; goto END; }
                }
            }
            while (cus.count(HUOGONG) > 0) {
                cus.removeCard(HUOGONG); deck.discardCard(HUOGONG);
                cout << "[火攻] 自定义使用";
                // 火攻无法打牌抵消，只有生命<=2时才用无懈
                if ((sq.count(WUXIE) > 0 && sq.hp <= 2)
                    || sq.count(WUXIE) >= 3) {
                    sq.removeCard(WUXIE); deck.discardCard(WUXIE);
                    cout << " → 孙权无懈" << endl;
                } else {
                    sq.hp--;
                    cout << " → 孙权受伤,HP=" << sq.hp << endl;
                    if (sq.hp <= 0) { cout << "孙权阵亡!" << endl; goto END; }
                }
            }
        }
        
        // 弃牌
        while (cus.totalCards() > cus.hp) {
            CardType c = pickDiscardForPhase(cus, sq.totalCards());
            deck.discardCard(c);
        }
        
        // 胜利判定
        int shanShaCount = cus.count(SHAN) + cus.getShaCount();
        if (shanShaCount > 40) {
            cout << "\n*** 自定义武将 杀+闪 > 40, 胜利! ***" << endl;
            break;
        }
        
        sq.showHand();
        cus.showHand();
        cout << "自定义 杀+闪 = " << shanShaCount << endl;
    }
    
    END:
    cout << "\n=== 模拟结束 ===" << endl;
    return 0;
}
